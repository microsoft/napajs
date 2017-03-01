#include "timeout-service.h"

#include <boost/asio/high_resolution_timer.hpp>

#include <thread>

#include <iostream>

using namespace napa::scheduler;

class TokenImpl : public TimeoutService::Token {
public:
    explicit TokenImpl(boost::asio::io_service& ioService) : timer(ioService) {}

    void Cancel() override {
        timer.cancel();
    }

    ~TokenImpl() override {
        timer.cancel();
    }

    boost::asio::high_resolution_timer timer;
};

struct TimeoutService::Impl {
    boost::asio::io_service ioService;
    std::unique_ptr<boost::asio::io_service::work> work;
    std::thread thread;
};

TimeoutService& TimeoutService::Instance() {
    static TimeoutService::Impl impl;
    static TimeoutService instance(impl);

    return instance;
}

TimeoutService::TimeoutService(Impl& impl) : _impl(impl) {
    _impl.work = std::make_unique<boost::asio::io_service::work>(impl.ioService);
    _impl.thread = std::thread([this]() {
        _impl.ioService.run();
    });
}

TimeoutService::~TimeoutService() {
    _impl.work.reset();
    _impl.ioService.stop();
    _impl.thread.join();
}

std::unique_ptr<TimeoutService::Token> TimeoutService::Register(
    std::chrono::milliseconds timeout,
    std::function<void(void)> callback) {

    auto token = std::make_unique<TokenImpl>(_impl.ioService);

    token->timer.expires_from_now(timeout);
    token->timer.async_wait([callback = std::move(callback)](const boost::system::error_code &err) {
        if (!err) {
            callback();
        }
    });

    return std::move(token);
}
