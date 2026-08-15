#pragma once

#include <asio/error.hpp>
#include <string_view>

namespace Cubed {

inline std::string_view net_error_message(const asio::error_code& ec) {
    if (!ec)
        return "No error.";

#define NET_CASE(err, msg)                                                     \
    if (ec == asio::error::err) {                                              \
        return msg;                                                            \
    }

    // ===== basic_errors =====

    NET_CASE(access_denied, "Access denied.")
    NET_CASE(address_family_not_supported, "Address family is not supported.")
    NET_CASE(address_in_use, "Address is already in use.")
    NET_CASE(already_connected, "Already connected.")
    NET_CASE(already_started, "Operation already in progress.")
    NET_CASE(broken_pipe, "Broken pipe.")
    NET_CASE(connection_aborted, "Connection aborted.")
    NET_CASE(connection_refused, "Connection refused by the remote host.")
    NET_CASE(connection_reset, "Connection reset by the remote host.")
    NET_CASE(bad_descriptor, "Invalid socket or file descriptor.")
    NET_CASE(fault, "Bad memory address.")
    NET_CASE(host_unreachable, "Host is unreachable.")
    NET_CASE(in_progress, "Operation is in progress.")
    NET_CASE(interrupted, "Operation interrupted.")
    NET_CASE(invalid_argument, "Invalid argument.")
    NET_CASE(message_size, "Message is too large.")
    NET_CASE(name_too_long, "Name is too long.")
    NET_CASE(network_down, "Network is down.")
    NET_CASE(network_reset, "Network connection was reset.")
    NET_CASE(network_unreachable, "Network is unreachable.")
    NET_CASE(no_descriptors, "Too many open sockets or files.")
    NET_CASE(no_buffer_space, "No buffer space available.")
    NET_CASE(no_memory, "Out of memory.")
    NET_CASE(no_permission, "Operation not permitted.")
    NET_CASE(no_protocol_option, "Protocol option is unavailable.")
    NET_CASE(no_such_device, "No such device.")
    NET_CASE(not_connected, "Socket is not connected.")
    NET_CASE(not_socket, "Object is not a socket.")
    NET_CASE(operation_aborted, "Operation cancelled.")
    NET_CASE(operation_not_supported, "Operation is not supported.")
    NET_CASE(shut_down, "Connection has been shut down.")
    NET_CASE(timed_out, "Operation timed out.")
    NET_CASE(try_again, "Temporary failure. Please try again.")
    NET_CASE(would_block, "Operation would block.")

    // ===== netdb_errors =====

    NET_CASE(host_not_found, "Host could not be resolved.")
    NET_CASE(host_not_found_try_again, "Temporary DNS resolution failure.")
    NET_CASE(no_data, "No address data available.")
    NET_CASE(no_recovery, "Non-recoverable DNS error.")

    // ===== addrinfo_errors =====

    NET_CASE(service_not_found, "Requested service was not found.")
    NET_CASE(socket_type_not_supported, "Socket type is not supported.")

    // ===== misc_errors =====

    NET_CASE(already_open, "Resource is already open.")
    NET_CASE(eof, "Connection closed by the remote host.")
    NET_CASE(not_found, "Requested item was not found.")
    NET_CASE(fd_set_failure, "Too many descriptors for select().")

#undef NET_CASE

    return "Unknown network error.";
}

} // namespace Cubed