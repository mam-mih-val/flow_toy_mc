#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H

#include <algorithm>
#include <functional>
#include <random>

#include <Math/Vector4D.h>
#include <TDatabasePDG.h>

#include "random_number_generator.h"

struct Particle{
  ROOT::Math::PxPyPzMVector momentum{};
  int pdg{};
};

struct ParticlGeneratorSetup{
public:
  ParticlGeneratorSetup() = default;

  auto SetPdg( int pdg )  -> ParticlGeneratorSetup& { pdg_ = pdg; return *this; }
  auto SetNParticlesDistribution( std::poisson_distribution<size_t> distr ) -> ParticlGeneratorSetup& { n_particles_ = distr; return *this; } ;
  auto SetYDistribution( std::normal_distribution< double > distr ) -> ParticlGeneratorSetup& { y_ = distr; return *this; }
  auto SetPtDistribution( std::normal_distribution< double > distr ) -> ParticlGeneratorSetup& { pT_ = distr; return *this; }
  auto SetV1PtYDependance( std::function<double(double, double)> func ) -> ParticlGeneratorSetup& { pT_y_dependance_ = func; return *this; }

private:
  int pdg_;
  std::poisson_distribution<size_t> n_particles_{};
  std::normal_distribution< double >  pT_{};
  std::normal_distribution< double > y_{};
  std::function<double(double, double)> pT_y_dependance_ {};

  friend class ParticleGenerator;
};

class ParticleGenerator{
public:
  ParticleGenerator( ParticlGeneratorSetup setup ) :
    pdg_{ setup.pdg_ },
    n_particles_generator_( setup.n_particles_ ),
    pT_geneator_( setup.pT_ ),
    rapidity_geneator_( setup.y_),
    phi_generator_( std::uniform_real_distribution<double>( -M_PI, M_PI ) ),
    unity_generator_{ std::uniform_real_distribution<double>( 0, 1 ) },
    v1_pT_y_dependence_( std::move(setup.pT_y_dependance_) ) {}
  
  ~ParticleGenerator() = default;
  ParticleGenerator( ParticleGenerator&& ) = default;
  ParticleGenerator& operator=( ParticleGenerator&& ) = default;

  auto Sample() -> std::vector<Particle> {
    auto n_particles = n_particles_generator_.SampleNumber();
    auto particles = std::vector<Particle>( n_particles );
    std::generate( particles.begin(), particles.end(), [this](){ return SampleParticle(); } );
    return particles;
  }

private:
  auto SampleParticle() -> Particle{
    auto particle = Particle();
    auto y = rapidity_geneator_.SampleNumber();
    auto px_tmp = pT_geneator_.SampleNumber();
    auto py_tmp = pT_geneator_.SampleNumber();
    auto pT = sqrt( px_tmp*px_tmp + py_tmp*py_tmp );
    auto v1 = v1_pT_y_dependence_( pT, y );
    auto phi = SamplePhi( v1 );
    auto mass = TDatabasePDG::Instance()->GetParticle( pdg_ )->Mass();

    auto px = pT*cos( phi );
    auto py = pT*sin( phi );
    auto pz = sqrt( pT*pT + mass*mass ) * sinh( y );

    particle.momentum = ROOT::Math::PxPyPzMVector{ px, py, pz, mass };
    particle.pdg = pdg_;

    return particle;
  }
  auto SamplePhi(double v1) -> double {
    auto phi = phi_generator_.SampleNumber();
    auto probability = PhiDensityFunction(phi, v1);
    auto number = unity_generator_.SampleNumber();
    if( number < probability )
      return phi;
    else
      return SamplePhi(v1);
  }
  auto PhiDensityFunction( double phi, double v1 ) const -> double{
    return 1 / ( 2*M_PI ) * ( 1 + 2 * v1 * cos(phi) );
  }

  int pdg_{};

  RandomNumberGenerator<std::poisson_distribution<size_t>> n_particles_generator_;
  RandomNumberGenerator< std::normal_distribution< double > > pT_geneator_;
  RandomNumberGenerator< std::normal_distribution< double > > rapidity_geneator_;
  RandomNumberGenerator< std::uniform_real_distribution<double> > phi_generator_;
  RandomNumberGenerator< std::uniform_real_distribution<double> > unity_generator_;

  std::function<double(double, double)> v1_pT_y_dependence_;
};

#endif // PARTICLE_GENERATOR_H