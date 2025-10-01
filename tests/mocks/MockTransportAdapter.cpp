#include "tests/mocks/MockTransportAdapter.h"
#include <algorithm>

namespace xuanyu {
namespace comm {

MockTransportAdapter::MockTransportAdapter() 
    : connected_(false), 
      remotePort_(0),
      totalBytesSent_(0),
      totalBytesReceived_(0),
      timeout_(std::chrono::milliseconds(5000)),
      sendBufferSize_(8192), 
      recvBufferSize_(8192),
      keepAlive_(false),
      lastErrorCode_(0),
      connectionDelay_(std::chrono::milliseconds(0)),
      sendDelay_(std::chrono::milliseconds(0)),
      receiveDelay_(std::chrono::milliseconds(0)),
      connectionShouldFail_(false),
      connectionFailureCode_(0),
      sendShouldFail_(false),
      sendFailureCode_(0),
      receiveShouldFail_(false),
      receiveFailureCode_(0) {
}

MockTransportAdapter::~MockTransportAdapter() {
    disconnect();
}

bool MockTransportAdapter::connect(const std::string& address, int port) {
    remoteAddress_ = address;
    remotePort_ = port;
    connected_ = true;
    lastErrorCode_ = 0;
    return true;
}

std::future<bool> MockTransportAdapter::connectAsync(const std::string& address, int port) {
    std::promise<bool> promise;
    promise.set_value(connect(address, port));
    return promise.get_future();
}

void MockTransportAdapter::disconnect() {
    connected_ = false;
    std::lock_guard<std::mutex> lock(dataQueueMutex_);
    while (!receiveQueue_.empty()) {
        receiveQueue_.pop();
    }
}

bool MockTransportAdapter::isConnected() const {
    return connected_;
}

int MockTransportAdapter::send(const void* data, size_t size) {
    if (!connected_) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    std::vector<uint8_t> vec(bytes, bytes + size);
    
    std::lock_guard<std::mutex> lock(dataQueueMutex_);
    sentData_.push_back(vec);
    
    return static_cast<int>(size);
}

int MockTransportAdapter::send(const std::vector<uint8_t>& data) {
    return send(data.data(), data.size());
}

std::future<int> MockTransportAdapter::sendAsync(const std::vector<uint8_t>& data) {
    std::promise<int> promise;
    promise.set_value(send(data));
    return promise.get_future();
}

int MockTransportAdapter::recv(void* buffer, size_t size) {
    if (!connected_) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(dataQueueMutex_);
    if (receiveQueue_.empty()) {
        return 0;
    }
    
    auto data = receiveQueue_.front();
    receiveQueue_.pop();
    
    size_t copySize = std::min(size, data.size());
    std::copy(data.begin(), data.begin() + copySize, static_cast<uint8_t*>(buffer));
    
    return static_cast<int>(copySize);
}

std::vector<uint8_t> MockTransportAdapter::recv(size_t maxSize) {
    std::vector<uint8_t> buffer(maxSize);
    int received = recv(buffer.data(), maxSize);
    if (received > 0) {
        buffer.resize(received);
        return buffer;
    }
    return {};
}

std::future<std::vector<uint8_t>> MockTransportAdapter::recvAsync(size_t maxSize) {
    std::promise<std::vector<uint8_t>> promise;
    promise.set_value(recv(maxSize));
    return promise.get_future();
}

int MockTransportAdapter::getLastErrorCode() const {
    return lastErrorCode_;
}

std::string MockTransportAdapter::getLastError() const {
    return lastErrorMessage_;
}

void MockTransportAdapter::setErrorCallback(const ErrorCallback& callback) {
    errorCallback_ = callback;
}

void MockTransportAdapter::setConnectionCallback(const ConnectionCallback& callback) {
    connectionCallback_ = callback;
}

void MockTransportAdapter::setDataCallback(const DataCallback& callback) {
    dataCallback_ = callback;
}

void MockTransportAdapter::setTimeout(std::chrono::milliseconds timeout) {
    timeout_ = timeout;
}

std::chrono::milliseconds MockTransportAdapter::getTimeout() const {
    return timeout_;
}

void MockTransportAdapter::setSendBufferSize(size_t size) {
    sendBufferSize_ = size;
}

void MockTransportAdapter::setRecvBufferSize(size_t size) {
    recvBufferSize_ = size;
}

size_t MockTransportAdapter::getSendBufferSize() const {
    return sendBufferSize_;
}

size_t MockTransportAdapter::getRecvBufferSize() const {
    return recvBufferSize_;
}

void MockTransportAdapter::setKeepAlive(bool enable) {
    keepAlive_ = enable;
}

void MockTransportAdapter::setKeepAliveInterval(std::chrono::seconds interval) {
    keepAliveInterval_ = interval;
}

std::string MockTransportAdapter::getLocalAddress() const {
    return "127.0.0.1";
}

int MockTransportAdapter::getLocalPort() const {
    return 12345;
}

std::string MockTransportAdapter::getRemoteAddress() const {
    return remoteAddress_;
}

int MockTransportAdapter::getRemotePort() const {
    return remotePort_;
}

std::chrono::system_clock::time_point MockTransportAdapter::getConnectedTime() const {
    return connectedTime_;
}

size_t MockTransportAdapter::getTotalBytesSent() const {
    std::lock_guard<std::mutex> lock(dataQueueMutex_);
    size_t total = 0;
    for (const auto& data : sentData_) {
        total += data.size();
    }
    return total;
}

size_t MockTransportAdapter::getTotalBytesReceived() const {
    return totalBytesReceived_;
}

// Mock-specific methods
void MockTransportAdapter::simulateReceiveData(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(dataQueueMutex_);
    receiveQueue_.push(data);
    totalBytesReceived_ += data.size();
}

std::vector<std::vector<uint8_t>> MockTransportAdapter::getSentData() const {
    std::lock_guard<std::mutex> lock(dataQueueMutex_);
    return sentData_;
}

void MockTransportAdapter::clearSentData() {
    std::lock_guard<std::mutex> lock(dataQueueMutex_);
    sentData_.clear();
}

} // namespace comm
} // namespace xuanyu