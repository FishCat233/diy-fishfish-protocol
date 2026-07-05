#include "socket.h"

#include <iostream>
#include <cstring>
#include <system_error>

#ifdef _WIN32

// ---------------------------------------------------------------------------
// ServiceGuard
// ---------------------------------------------------------------------------

ServiceGuard &ServiceGuard::getInstance() {
    static ServiceGuard instance;
    return instance;
}

ServiceGuard::ServiceGuard() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to initialize winsock");
    }
}

ServiceGuard::~ServiceGuard() {
    if (WSACleanup() != 0) {
        std::cerr << "Failed to cleanup winsock" << std::endl;
    }
}

// ---------------------------------------------------------------------------
// InetAddress
// ---------------------------------------------------------------------------

InetAddress::InetAddress() {
    std::memset(&m_addr, 0, sizeof(m_addr));
    m_len = sizeof(m_addr);
}

InetAddress::InetAddress(const std::string &ip, uint16_t port) {
    std::memset(&m_addr, 0, sizeof(m_addr));

    auto *addr4 = reinterpret_cast<sockaddr_in *>(&m_addr);
    if (inet_pton(AF_INET, ip.c_str(), &addr4->sin_addr) == 1) {
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        m_len = sizeof(struct sockaddr_in);
        return;
    }

    auto *addr6 = reinterpret_cast<sockaddr_in6 *>(&m_addr);
    if (inet_pton(AF_INET6, ip.c_str(), &addr6->sin6_addr) == 1) {
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        m_len = sizeof(struct sockaddr_in6);
        return;
    }

    throw std::runtime_error("Invalid IP address");
}

const struct sockaddr *InetAddress::getSockAddr() const {
    return reinterpret_cast<const struct sockaddr *>(&m_addr);
}

struct sockaddr *InetAddress::getSockAddr() {
    return reinterpret_cast<struct sockaddr *>(&m_addr);
}

socklen_t InetAddress::getLen() const {
    return m_len;
}

void InetAddress::setLen(socklen_t len) {
    m_len = len;
}

InetAddressType InetAddress::getType() const {
    switch (m_addr.ss_family) {
    case AF_INET:  return InetAddressType::IPv4;
    case AF_INET6: return InetAddressType::IPv6;
    default:
        throw std::runtime_error("Invalid address type");
    }
}

InetAddress InetAddress::any(InetAddressType type, uint16_t port) {
    switch (type) {
    case InetAddressType::IPv4: return InetAddress("0.0.0.0", port);
    case InetAddressType::IPv6: return InetAddress("::", port);
    default:
        throw std::runtime_error("Invalid address type");
    }
}

// ---------------------------------------------------------------------------
// Socket
// ---------------------------------------------------------------------------

Socket::Socket(SocketType type, InetAddress addr) {
    ServiceGuard::getInstance();

    SOCKET sock;
    switch (type) {
    case SocketType::TCP:
        sock = socket(static_cast<int>(addr.getType()), SOCK_STREAM, 0);
        break;
    case SocketType::UDP:
        sock = socket(static_cast<int>(addr.getType()), SOCK_DGRAM, 0);
        break;
    default:
        throw std::runtime_error("Invalid socket type");
    }

    if (sock == INVALID_SOCKET) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to create socket");
    }

    m_sock = sock;
}

Socket::Socket(Socket &&rhs) {
    if (this != &rhs) {
        m_sock = rhs.m_sock;
        rhs.m_sock = INVALID_SOCKET;
    }
}

Socket &Socket::operator=(Socket &&rhs) {
    if (this != &rhs) {
        close();
        m_sock = rhs.m_sock;
        rhs.m_sock = INVALID_SOCKET;
    }
    return *this;
}

Socket::~Socket() {
    close();
}

void Socket::bindAddress(InetAddress addr) {
    if (bind(m_sock, addr.getSockAddr(), addr.getLen()) == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to bind socket");
    }
}

Socket Socket::acceptSocket(InetAddress &addr) {
    socklen_t addrLen = addr.getLen();
    SOCKET sock = accept(m_sock, addr.getSockAddr(), &addrLen);
    if (sock == INVALID_SOCKET) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to accept socket");
    }
    addr.setLen(addrLen);
    return Socket(sock);
}

void Socket::close() noexcept {
    if (m_sock == INVALID_SOCKET) return;
    if (closesocket(m_sock) == SOCKET_ERROR) {
        std::cerr << "Failed to close socket: " << WSAGetLastError() << std::endl;
    }
}

Socket::Socket(SOCKET sock) : m_sock(sock) {}

// ---------------------------------------------------------------------------
// Listener / Receiver (stubs)
// ---------------------------------------------------------------------------

Listener::Listener(SocketType type, InetAddress addr, int backlog)
    : sock(type, addr) {}

Receiver::Receiver(Socket &&sock) : sock(std::move(sock)) {}

#endif  // _WIN32
