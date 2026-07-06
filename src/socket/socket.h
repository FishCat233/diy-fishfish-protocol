#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

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
    Socket(SocketType type, InetAddressType addrType = InetAddressType::IPv4);
    ~Socket();

    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;

    Socket(Socket &&rhs);
    Socket &operator=(Socket &&rhs);

    void bind(InetAddress addr);
    void listen(int backlog = SOMAXCONN);
    Socket accept(InetAddress &addr);

    void send(const void *buf, size_t len);
    void recv(void *buf, size_t len);

    template <typename T>
    void send(const T &value);

    template <typename T>
    void recv(T &value);

    void setOption(int level, int optionName, const void *optionValue, socklen_t optionLen);
    template <typename T>
    void setOption(int level, int optionName, const T &value);

    // TODO: it would be good if there are have some more easier option setter methods, such as `setReuseAddr(bool)`.
    // but im lazy and it just can work now. i wont implement it now.

    explicit operator bool() const noexcept;
    bool operator!() const noexcept;

private:
    SOCKET m_sock;

    void close() noexcept;
    explicit Socket(SOCKET sock);
};

// TODO: pending implementation
class Listener {
public:
    Listener(SocketType type, const InetAddress &addr, int backlog);
    ~Listener();
    Listener(const Listener &) = delete;
    Listener &operator=(const Listener &) = delete;
    Listener(Listener &&);
    Listener &operator=(Listener &&);

    explicit operator bool() const noexcept;
    bool operator!() const noexcept;

    // TODO: docs
    // TODO: TIMEOUT IS NOT IMPLEMENT NOW.
    Connection accept(int timeout = -1);

private:
    Socket sock;
};

// TODO: pending implementation
class Connection {
public:
    explicit Connection(Socket &&sock, InetAddress &&addr);

    template <typename T>
    void send(const T &value);

    template <typename T>
    void recv(T &value);

private:
    // TODO: CACHE FOR RECEIVE AND SEND 'CAUSE TCP JUST SEND A VEEEEEERY SMALL AMOUNT OF DATA AT ONCE :(
    Socket sock;
    InetAddress addr;
};

#else // !_WIN32

// TODO: Linux / macOS support
class Socket {
public:
private:
    int sock;
};

#endif

/// FIXME: THERE IS NO LINUX / UNIX VERSION 'CAUSE IM LAZY AND I HAVENT A LINXU DEVICE.
/// SO PRs ARE WELCOME :)
