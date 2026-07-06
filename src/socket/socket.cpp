#include "socket.h"

#include <cstring>
#include <iostream>
#include <system_error>

#ifdef _WIN32

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
    case AF_INET:
        return InetAddressType::IPv4;
    case AF_INET6:
        return InetAddressType::IPv6;
    default:
        throw std::runtime_error("Invalid address type");
    }
}

InetAddress InetAddress::any(InetAddressType type, uint16_t port) {
    switch (type) {
    case InetAddressType::IPv4:
        return InetAddress("0.0.0.0", port);
    case InetAddressType::IPv6:
        return InetAddress("::", port);
    default:
        throw std::runtime_error("Invalid address type");
    }
}

Socket::Socket(SocketType type, InetAddressType addrType = InetAddressType::IPv4) {
    ServiceGuard::getInstance();

    SOCKET sock;
    switch (type) {
    case SocketType::TCP:
        sock = socket(static_cast<int>(addrType), SOCK_STREAM, 0);
        break;
    case SocketType::UDP:
        sock = socket(static_cast<int>(addrType), SOCK_DGRAM, 0);
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

void Socket::bind(InetAddress addr) {
    if (::bind(m_sock, addr.getSockAddr(), addr.getLen()) == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to bind socket");
    }
}

void Socket::listen(int backlog = SOMAXCONN) {
    if (::listen(m_sock, backlog) == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to listen socket");
    }
}

Socket Socket::accept(InetAddress &addr) {
    socklen_t addrLen = addr.getLen();
    SOCKET sock = ::accept(m_sock, addr.getSockAddr(), &addrLen);
    if (sock == INVALID_SOCKET) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to accept socket");
    }
    addr.setLen(addrLen);
    return Socket(sock);
}

void Socket::send(const void *buf, size_t len) {
    if (::send(m_sock, reinterpret_cast<const char *>(buf), len, 0) == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to send data");
    }
}

void Socket::recv(void *buf, size_t len) {
    if (::recv(m_sock, reinterpret_cast<char *>(buf), len, 0) == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to receive data");
    }
}

void Socket::setOption(int level, int optionName, const void *optionValue, socklen_t optionLen) {
    if (setsockopt(m_sock, level, optionName, static_cast<const char *>(optionValue), optionLen) == SOCKET_ERROR) {
        throw std::system_error(
            WSAGetLastError(),
            std::system_category(),
            "Failed to set socket option");
    }
}

Socket::operator bool() const noexcept {
    return m_sock != INVALID_SOCKET;
}

bool Socket::operator!() const noexcept {
    return m_sock == INVALID_SOCKET;
}

void Socket::close() noexcept {
    if (m_sock == INVALID_SOCKET)
        return;
    if (closesocket(m_sock) == SOCKET_ERROR) {
        std::cerr << "Failed to close socket: " << WSAGetLastError() << std::endl;
    }
}

Socket::Socket(SOCKET sock) : m_sock(sock) {}

Listener::Listener(SocketType type, const InetAddress &addr, int backlog)
    : sock(type, addr.getType()) {
    sock.bind(addr);
    sock.listen(backlog);
}

Listener::~Listener() {}
Listener::Listener(Listener &&rhs) : sock(std::move(rhs.sock)) {}
Listener &Listener::operator=(Listener &&rhs) {
    if (this != &rhs) {
        sock = std::move(rhs.sock);
    }
    return *this;
}

Listener::operator bool() const noexcept {
    return sock.operator bool();
}

bool Listener::operator!() const noexcept {
    return sock.operator!();
}

Connection Listener::accept(int timeoutMs) {
    InetAddress addr;
    auto connectSock = sock.accept(addr);
    return Connection(std::move(connectSock), std::move(addr));
}

Connection::Connection(Socket &&sock, InetAddress &&addr) : sock(std::move(sock)), addr(std::move(addr)) {}

#endif // _WIN32
