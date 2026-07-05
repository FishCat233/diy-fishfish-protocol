#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

// RAII wrapper for WSAStartup / WSACleanup (singleton)
class ServiceGuard {
public:
    static ServiceGuard &getInstance();

    ServiceGuard &operator=(const ServiceGuard &) = delete;
    ServiceGuard &operator=(ServiceGuard &&) = delete;
    ServiceGuard(const ServiceGuard &) = delete;
    ServiceGuard(ServiceGuard &&) = delete;

private:
    ServiceGuard();
    ~ServiceGuard();
};

enum class InetAddressType {
    IPv4 = AF_INET,
    IPv6 = AF_INET6
};

// Wraps sockaddr_storage; supports IPv4 and IPv6
class InetAddress {
public:
    InetAddress();
    InetAddress(const std::string &ip, uint16_t port);

    InetAddress(const InetAddress &rhs) = default;
    InetAddress &operator=(const InetAddress &rhs) = default;
    InetAddress &operator=(InetAddress &&rhs) = default;

    const struct sockaddr *getSockAddr() const;
    struct sockaddr *getSockAddr();
    socklen_t getLen() const;
    void setLen(socklen_t len);
    InetAddressType getType() const;

    static InetAddress any(InetAddressType type, uint16_t port);

private:
    struct sockaddr_storage m_addr;
    socklen_t m_len;
};

enum class SocketType {
    TCP,
    UDP
};

// Move-only socket wrapper
class Socket {
public:
    Socket(SocketType type, InetAddress addr);
    ~Socket();

    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;

    Socket(Socket &&rhs);
    Socket &operator=(Socket &&rhs);

    void bindAddress(InetAddress addr);
    Socket acceptSocket(InetAddress &addr);

private:
    SOCKET m_sock;

    void close() noexcept;
    explicit Socket(SOCKET sock);
};

// TODO: pending implementation
class Listener {
public:
    Listener(SocketType type, InetAddress addr, int backlog);
private:
    Socket sock;
};

// TODO: pending implementation
class Receiver {
public:
    explicit Receiver(Socket &&sock);
private:
    Socket sock;
};

#else  // !_WIN32

// TODO: Linux / macOS support
class Socket {
public:
private:
    int sock;
};

#endif
