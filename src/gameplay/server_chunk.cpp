#include "Cubed/gameplay/server_chunk.hpp"

#include "Cubed/tools/cubed_assert.hpp"

namespace Cubed {
ServerChunk::ServerChunk(ServerWorld& world, ChunkPos chunk_pos,
                         bool temp_chunk)
    : m_temp_chunk(temp_chunk), m_chunk_pos(chunk_pos), m_world(world) {}

ServerChunk::ServerChunk(ServerChunk&& other) noexcept
    : m_gening(other.m_gening.load()), m_has_cave(other.m_has_cave),
      m_biome(other.m_biome.load()), m_chunk_pos(std::move(other.m_chunk_pos)),
      m_world(other.m_world), m_heightmap(std::move(other.m_heightmap)),
      m_blocks(std::move(other.m_blocks)),
      m_neightbor_blocks(std::move(other.m_neightbor_blocks)),
      m_seed(other.m_seed), m_conditions(other.m_conditions) {
    ASSERT_MSG(!other.m_gening, "Other is Gening Can't Move");
}

ServerChunk& ServerChunk::operator=(ServerChunk&& other) noexcept {
    // Logger::info("other Chunk pos {} {} in Chunk& Chunk::operator=(Chunk&&
    // other) this {}", other.m_chunk_pos.x, other.m_chunk_pos.z,
    // static_cast<const void*>(&other));
    if (this == &other) {
        return *this;
    }
    ASSERT_MSG(!other.m_gening, "Other is Gening Can't Move");
    m_chunk_pos = std::move(other.m_chunk_pos);
    m_heightmap = std::move(other.m_heightmap);
    m_blocks = std::move(other.m_blocks);
    m_biome = other.m_biome.load();
    m_seed = other.m_seed;
    m_conditions = other.m_conditions;
    m_neightbor_blocks = std::move(other.m_neightbor_blocks);
    m_has_cave = other.m_has_cave;
    m_gening = other.m_gening.load();
    return *this;
}

std::tuple<int, int, int> ServerChunk::world_to_block(int world_x, int world_y,
                                                      int world_z, int chunk_x,
                                                      int chunk_z) {
    int x, y, z;
    y = world_y;
    x = world_x - chunk_x * CHUNK_SIZE;
    z = world_z - chunk_z * CHUNK_SIZE;
    return {x, y, z};
}

std::tuple<int, int, int>
ServerChunk::world_to_block(const glm::ivec3& block_pos, ChunkPos chunk_pos) {
    return world_to_block(block_pos.x, block_pos.y, block_pos.z, chunk_pos.x,
                          chunk_pos.z);
}

std::tuple<int, int, int>
ServerChunk::block_to_world(int x, int y, int z, int chunk_x, int chunk_z) {
    int world_x = x + chunk_x * CHUNK_SIZE;
    int world_z = z + chunk_z * CHUNK_SIZE;
    int world_y = y;
    return {world_x, world_y, world_z};
}
std::tuple<int, int, int>
ServerChunk::block_to_world(const glm::ivec3& block_pos, ChunkPos chunk_pos) {
    return block_to_world(block_pos.x, block_pos.y, block_pos.z, chunk_pos.x,
                          chunk_pos.z);
}

BiomeType ServerChunk::get_biome() const { return m_biome.load(); }

ChunkPos ServerChunk::get_chunk_pos() const { return m_chunk_pos; }

const std::vector<BlockType>& ServerChunk::get_chunk_blocks() const {
    return m_blocks;
}

HeightMapArray ServerChunk::get_heightmap() const {
    // Logger::info("Chunk pos {} {} in get_heightmap this {}", m_chunk_pos.x,
    // m_chunk_pos.z, static_cast<const void*>(this));
    return m_heightmap;
}

int ServerChunk::index(int x, int y, int z) {
    ASSERT(!(x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= WORLD_SIZE_Y ||
             z >= CHUNK_SIZE));
    if ((x * WORLD_SIZE_Y + y) * CHUNK_SIZE + z < 0 ||
        (x * WORLD_SIZE_Y + y) * CHUNK_SIZE + z >=
            CHUNK_SIZE * CHUNK_SIZE * WORLD_SIZE_Y) {
        Logger::error("block pos x {} y {} z {} range error", x, y, z);
        ASSERT(0);
    }
    return (x * WORLD_SIZE_Y + y) * CHUNK_SIZE + z;
}

int ServerChunk::index(const glm::vec3& pos) {
    return ServerChunk::index(pos.x, pos.y, pos.z);
}

void ServerChunk::gen_phase_one() {
    m_generator = std::make_unique<ChunkGenerator>(*this);
    if (!m_generator) {
        Logger::error("ChunkGenerator is Nullptr");
        return;
    }
    m_generator->assign_chunk_biome();
    m_seed = m_generator->chunk_seed();
}
void ServerChunk::gen_phase_two() {
    if (!m_generator) {
        Logger::error("ChunkGenerator is Nullptr");
        return;
    }
    m_generator->generate_heightmap();
}
void ServerChunk::gen_phase_three() {
    if (!m_generator) {
        Logger::error("ChunkGenerator is Nullptr");
        return;
    }
    m_generator->generate_terrain_blocks();
}

void ServerChunk::gen_phase_four(
    const std::array<std::optional<std::vector<BlockType>>, 4>&
        neighbor_block) {
    if (!m_generator) {
        Logger::error("ChunkGenerator is Nullptr");
        return;
    }
    // This must be fully completed before any other operations can proceed!
    m_generator->blend_surface_blocks_borders(neighbor_block);
}

void ServerChunk::gen_phase_five() {
    if (!m_generator) {
        Logger::error("ChunkGenerator is Nullptr");
        return;
    }
    m_generator->ocean_build();
    m_generator->generate_river();
    m_generator->generate_cave();

    m_generator->generate_vegetation();
    m_generator = nullptr;
}

void ServerChunk::gen_chunk() {
    if (m_gening.exchange(true))
        return;
    m_gening = true;
    ASSERT_MSG(m_blocks.empty(),
               "Blocks isn't Empty, chunk already generated!");
    if (m_blocks.size() != 0) {
        Logger::warn(
            "Request Generator Chunk {} {} ,but the Blocks size is Not 0",
            m_chunk_pos.x, m_chunk_pos.z);
        return;
    }
    std::vector<ServerChunk> neighbor;
    for (int i = 0; i < 4; i++) {
        neighbor.emplace_back(m_world, m_chunk_pos + CHUNK_DIR[i], true);
    }
    for (auto& chunk : neighbor) {
        chunk.gen_phase_one();
        chunk.gen_phase_two();
        chunk.gen_phase_three();
        chunk.gen_phase_five();
    }
    gen_phase_one();
    gen_phase_two();
    gen_phase_three();

    for (int i = 0; i < 4; i++) {
        m_neightbor_blocks[i] = neighbor[i].get_chunk_blocks();
    }
    gen_phase_four(m_neightbor_blocks);
    gen_phase_five();
    m_gening = false;
}
// Logger::info("Cross Sum {}", m_cross_vertices_sum.load());

bool ServerChunk::is_temp_chunk() const { return m_temp_chunk.load(); }

bool& ServerChunk::has_cave() { return m_has_cave; }

const OptionalBlockVectorArray& ServerChunk::get_neightbor_blocks() const {
    return m_neightbor_blocks;
}

void ServerChunk::set_chunk_block(int index, unsigned id) {
    m_blocks[index] = id;
}
ChunkPos ServerChunk::chunk_pos() const { return m_chunk_pos; }

BiomeType ServerChunk::biome() const { return m_biome; }

void ServerChunk::biome(BiomeType b) { m_biome = b; }

HeightMapArray& ServerChunk::heightmap() { return m_heightmap; }
std::vector<BlockType>& ServerChunk::blocks() { return m_blocks; }
ServerWorld& ServerChunk::world() { return m_world; }
unsigned ServerChunk::seed() const {
    if (m_seed == 0) {
        Logger::warn("Seed Not Generator");
    }
    return m_seed;
}

BiomeConditions& ServerChunk::conditions() { return m_conditions; }

} // namespace Cubed