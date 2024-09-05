#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H

#include <mutex>
#include <random>

template<typename Distribution >
class RandomNumberGenerator{
public:
  RandomNumberGenerator(Distribution distr) : distribuition_( std::move(distr) ) {
    auto device = std::random_device{};
    rnd_number_generator_.seed( device() );
  }
  ~RandomNumberGenerator() = default;
  RandomNumberGenerator(RandomNumberGenerator&& other) noexcept { 
    auto device = std::random_device{};
    rnd_number_generator_.seed( device() );
    distribuition_ = std::move(other.distribuition_);
  };
  RandomNumberGenerator& operator=(RandomNumberGenerator&& other) noexcept {
    auto device = std::random_device{};
    rnd_number_generator_.seed( device() );
    distribuition_ = std::move(other.distribuition_);
  };
  auto SampleNumber() { 
    auto lock = std::lock_guard{m};
    return distribuition_( rnd_number_generator_ ); 
  }
private:
  std::mutex m{};
  std::mt19937 rnd_number_generator_;
  Distribution distribuition_;
};


#endif // RANDOM_NUMBER_GENERATOR_H