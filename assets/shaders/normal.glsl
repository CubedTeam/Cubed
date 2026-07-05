vec3 calcNewNormal() {
    mat3 TBN = mat3(normalize(tangent), normalize(bitangent), normalize(normal));
    vec3 retrievedNormal = texture(normMap, vec3(tc, tex_layer)).xyz;       
    retrievedNormal = retrievedNormal * 2.0 - 1.0;
    if (flipY) {
        retrievedNormal.y = -retrievedNormal.y;
    }
    vec3 newNormal = TBN * retrievedNormal;
    return normalize(newNormal);
}