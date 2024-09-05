#ifndef COLLISION_POOL_H
#define COLLISION_POOL_H

#include <optional>
#include <queue>
#include <mutex>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

#include "flow_analyzer.h"
#include "paritlce_generator.h"

#include "generator.h"

class CollisionPool{
public:
  CollisionPool( size_t n_req ) { n_required_ = n_req; };

  auto Add(const Collision& col) -> void { 
    auto lock = std::lock_guard{ m }; 
    pool_.push( col );
    n_generated_++;
  }
  
  auto Get() -> std::optional<Collision> {
    auto lock = std::lock_guard{ m }; 
    auto col = !pool_.empty() ? std::make_optional( pool_.front() ) : std::optional<Collision>{};
    if( !col.has_value() )
      return col;
    n_analyzed_++;
    pool_.pop();
    return col;
  }
  
  auto Run(){
    auto generator_threads = SpawnGenerators();
    auto analysis_thread = SpawnAnalyzer();
    for( auto& thr : generator_threads )
      thr.join();
    analysis_thread.join();
  }

  auto GetNEvents(){ return n_generated_.load(); }
  auto GetNAnalyzed(){ return n_generated_.load(); }

  auto AddGenerator( Generator gen ){ generators_.push_back(std::move(gen) ); }
  auto SetAnalyzer( CollisionAnalyzer&& anal ){ analyzer_ = std::move(anal); }

private:
  auto SpawnGenerators() -> std::vector<std::thread> {
    std::vector<std::thread> threads{};
    for( auto& gen : generators_ ){
      threads.emplace_back(  
        [&gen, this](){ 
          while ( n_generated_ < n_required_ ){ Add( gen.GenerateCollision() ); } 
        }
      );
    }
    return threads;
  }
  auto SpawnAnalyzer() -> std::thread{
    auto analysis_thread = std::thread{
      [this](){
        while (n_analyzed_ < n_required_) {
          auto col = Get();
          if( ! col.has_value() ) 
            continue;
          analyzer_.Analyze( col.value() ); 
        }
        analyzer_.Finalyze();
      }
    };
    return analysis_thread;
  }

  std::mutex m;
  std::queue<Collision> pool_{};
  std::atomic<size_t> n_required_{};
  std::atomic<size_t> n_generated_{};
  std::atomic<size_t> n_analyzed_{};

  std::vector<Generator> generators_{};
  CollisionAnalyzer analyzer_{};
};

#endif // COLLISION_POOL_H