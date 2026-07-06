#pragma once

#include "socket.h"

#include <system_error>

#ifdef _WIN32
template <typename T>
inline void Socket::send(const T &value) {
    if (::send(m_sock, reinterpret_cast<const char *>(&value), sizeof(T), 0) == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to send data");
    }
}

template <typename T>
inline void Socket::recv(T &value) {
    int bytes = ::recv(m_sock, reinterpret_cast<char *>(&value), sizeof(T), 0);

    if (bytes == 0) {
        throw std::system_error,
            std::make_error_code(std::errc::connection_aborted),
            "Connection aborted";
    }

    else if (bytes == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to receive data");
    }
}

template <typename T>
inline void Socket::setOption(int level, int optionName, const T &value) {
    if (::setsockopt(m_sock, level, optionName, reinterpret_cast<const char *>(&value), sizeof(T)) == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to set socket option");
    }
}

template <typename T>
inline void Connection::send(const T &value) {
    // FIXME: THIS TEMPLATE WILL BUG 'CAUSE PADDING / ENDIANNESS / ALIGNMENT. FK IT.
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    sock.send(value);
}

template <typename T>
inline void Connection::recv(T &value) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    sock.recv(value);
}

#endif // _WIN33