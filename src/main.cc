
#include <TROOT.h>
#include <TFile.h>
#include "paritlce_generator.h"
#include "generator.h"
#include "collision_pool.h"
#include "flow_analyzer.h"
#include "random_resources.h"

#include <random>
#include <iostream>
#include <thread>

auto main() -> int {
  ROOT::EnableThreadSafety();

  auto pool = CollisionPool{ 1000000 };

  auto resources = RandomResources{};

  auto v1_amp = double{ 0.5 };

  resources
  // protons
    .AddParticleGenerator(ParticlGeneratorSetup()
      .SetPdg(2212)
      .SetNParticlesDistribution( std::poisson_distribution<size_t>{ 50 } )
      .SetPtDistribution( std::normal_distribution<double>{ 0.0, 0.5 } )
      .SetYDistribution( std::normal_distribution<double>{ 0.0, 0.5 } ) 
      .SetV1PtYDependance( [v1_amp]( double pT, double y ){ return v1_amp * y  * pT; } ))
    .AddParticleGenerator(ParticlGeneratorSetup()
      .SetPdg(2212)
      .SetNParticlesDistribution( std::poisson_distribution<size_t>{ 25 } )
      .SetPtDistribution( std::normal_distribution<double>{ 0.0, 0.1 } )
      .SetYDistribution( std::normal_distribution<double>{ 1.0, 0.1 } ) 
      .SetV1PtYDependance( [v1_amp]( double pT, double y ){ return v1_amp * y  * pT; } ) )
    .AddParticleGenerator(ParticlGeneratorSetup()
      .SetPdg(2212)
      .SetNParticlesDistribution( std::poisson_distribution<size_t>{ 25 } )
      .SetPtDistribution( std::normal_distribution<double>{ 0.0, 0.1 } )
      .SetYDistribution( std::normal_distribution<double>{ -1.0, 0.1 } ) 
      .SetV1PtYDependance( [v1_amp]( double pT, double y ){ return v1_amp * y  * pT; } ) )
    //neutrons
    .AddParticleGenerator(ParticlGeneratorSetup()
      .SetPdg(2112)
      .SetNParticlesDistribution( std::poisson_distribution<size_t>{ 100 } )
      .SetPtDistribution( std::normal_distribution<double>{ 0.0, 0.5 } )
      .SetYDistribution( std::normal_distribution<double>{ 0.0, 0.5 } ) 
      .SetV1PtYDependance( [v1_amp]( double pT, double y ){ return v1_amp * y  * pT; } )
    )
    .AddParticleGenerator(ParticlGeneratorSetup()
      .SetPdg(2112)
      .SetNParticlesDistribution( std::poisson_distribution<size_t>{ 50 } )
      .SetPtDistribution( std::normal_distribution<double>{ 0.0, 0.1 } )
      .SetYDistribution( std::normal_distribution<double>{ 1.0, 0.1 } ) 
      .SetV1PtYDependance( [v1_amp]( double pT, double y ){ return v1_amp * y  * pT; } ) 
    )
    .AddParticleGenerator(ParticlGeneratorSetup()
      .SetPdg(2112)
      .SetNParticlesDistribution( std::poisson_distribution<size_t>{ 50 } )
      .SetPtDistribution( std::normal_distribution<double>{ 0.0, 0.1 } )
      .SetYDistribution( std::normal_distribution<double>{ -1.0, 0.1 } ) 
      .SetV1PtYDependance( [v1_amp]( double pT, double y ){ return v1_amp * y  * pT; } ) 
    )
    ;
    
  auto gen1 = Generator{};

  resources.Register( gen1 );

  auto analyzer = CollisionAnalyzer{};
 
  pool.AddGenerator(gen1 );
  pool.SetAnalyzer(std::move(analyzer));

  pool.Run();
  
  std::cout << pool.GetNEvents() << std::endl;

  return 0;
}