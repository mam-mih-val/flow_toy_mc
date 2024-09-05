#ifndef FLOW_ANALYZER_H
#define FLOW_ANALYZER_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <math.h>
#include <memory>
#include <random>
#include <vector>

#include <TProfile.h>
#include <TH1F.h>
#include <TH2F.h>

#include "collision_pool.h"
#include "generator.h"
#include "random_number_generator.h"

struct uvector{
  double phi{};

  auto Fill( double p ){
    phi = p;
  }
  auto x( size_t n=1 ) const {
    return cos( n*phi );
  }
  auto y( size_t n=1 ) const {
    return sin( n*phi );
  }
};

struct Qvector{
  std::array<double, 2> sumx{};
  std::array<double, 2> sumy{};
  size_t entries{};
  
  auto Fill( const uvector& u ){
    sumx.at(0)+=u.x(1);
    sumy.at(0)+=u.y(1);

    sumx.at(1)+=u.x(2);
    sumy.at(1)+=u.y(2);

    entries++;
  }
  auto x(size_t n=1) const { return entries > 0 ? sumx.at(n-1)/entries : 0 ; }
  auto y(size_t n=1) const { return entries > 0 ? sumy.at(n-1)/entries : 0; }
};

struct MeanCalculator{
  double sum_value{};
  double n_entries{};
  auto Fill( double val ){ sum_value += val; n_entries+=1; }
  auto Mean() {
    return sum_value / n_entries;
  } 
};

class CollisionAnalyzer{
public:
  CollisionAnalyzer() {
    v1_psi_rp_rapidity_.reset( new TProfile( "v1_psi_rp", ";y", 20, -1.0, 1.0 ));
    v1_psi_sp_rapidity_.reset( new TProfile( "v1_psi_sp", ";y", 20, -1.0, 1.0 ));
    v1_psi_half_rapidity_.reset( new TProfile( "v1_half", ";y", 20, -1.0, 1.0 ));
    
    h1_y_.reset(new TH1F( "h1_y", ";y", 200, -2.0, 2.0 ));
    h1_pT_.reset( new TH1F( "h1_pT", ";pT", 300, 0.0, 3.0 ));
    
    h1_Qx_.reset(new TH1F( "h1_Qx", ";y", 200, -2.0, 2.0 ));
    h1_Qy_.reset(new TH1F( "h1_Qy", ";y", 200, -2.0, 2.0 ));
  };
  
  auto Analyze( [[maybe_unused]] const Collision& collision ) -> void {
    auto psi_rp =  uvector{};
    psi_rp.Fill(collision.reaction_plane);
    auto full_2112 = Qvector{};
    auto half_2112 = Qvector{};
    auto num_gen = RandomNumberGenerator{ std::uniform_real_distribution<double>{0.0, 1.0} };
    std::for_each( collision.particles.begin(), collision.particles.end(), [&, i=0](const auto& p) mutable {
      h1_y_->Fill( p.momentum.Rapidity() );
      h1_pT_->Fill( p.momentum.Pt() );
      
      auto u = uvector{};
      u.Fill(p.momentum.Phi());
      
      auto uxQx = u.x() * psi_rp.x();
      if( p.momentum.Pt() > 0.5 && p.pdg == 2212 )
        v1_psi_rp_rapidity_->Fill( p.momentum.Rapidity(), uxQx );

      ++i;
      if( p.pdg != 2112 )
        return;
      if( p.momentum.Rapidity() < 0.9 )    
        return;
      full_2112.Fill(u);
      auto num = num_gen.SampleNumber();
      if( num < 0.025 )
        half_2112.Fill(u);
    } );

    res_psi_sp_.Fill(full_2112.x()*psi_rp.x() );
    res_half_.Fill(half_2112.x()*psi_rp.x() );

    std::for_each( collision.particles.begin(), collision.particles.end(), [&, i=0](const auto& p) mutable {
      auto u = uvector{};
      u.Fill(p.momentum.Phi() );
      
      auto uxQx = u.x() * full_2112.x();
      if( p.momentum.Pt() > 0.5 && p.pdg == 2212 ){
        v1_psi_sp_rapidity_->Fill( p.momentum.Rapidity(), uxQx );
        v1_psi_half_rapidity_->Fill( p.momentum.Rapidity(), u.x() * half_2112.x() );
      }
      ++i;
    } );
    
  }

  auto Finalyze(){
    v1_psi_rp_rapidity_->Scale(2.0);
    // v1_psi_sp_rapidity_->Scale(1.0 / res_psi_sp_.Mean() );
    
    std::cout << res_psi_sp_.Mean() << "\n";
    
    auto ratio = res_psi_sp_.Mean() / res_half_.Mean();
    std::cout << ratio << std::endl;

    v1_psi_sp_rapidity_->Scale( 1.0/res_psi_sp_.Mean() );
    v1_psi_half_rapidity_->Scale( 1.0/res_half_.Mean() );

    auto nf_full = dynamic_cast<TProfile*>(v1_psi_sp_rapidity_->Clone( "nf_full" ));
    auto nf_half = dynamic_cast<TProfile*>(v1_psi_half_rapidity_->Clone( "nf_half" ));
    nf_full->Add( v1_psi_rp_rapidity_.get(), v1_psi_sp_rapidity_.get(), 1, -1 );
    nf_half->Add( v1_psi_rp_rapidity_.get(), v1_psi_half_rapidity_.get(), 1, -1 );

    // v1_nonflow_estimated_->Add( v1_psi_half_rapidity_.get(), v1_psi_sp_rapidity_.get(), ratio, -1 );
    // v1_nonflow_estimated_->Scale( 1.0 / (ratio - 1) );
    auto file_out = TFile::Open( str_out_file_.c_str(), "RECREATE" );
    v1_psi_rp_rapidity_->Write();
    v1_psi_sp_rapidity_->Write();
    v1_psi_half_rapidity_->Write();
    nf_full->Write();
    nf_half->Write();
    h1_y_->Write();
    h1_pT_->Write();
    h1_Qx_->Write();
    h1_Qy_->Write();
    file_out->Close();
  }
private:
  std::string str_out_file_{ "file.root" };

  std::unique_ptr<TProfile> v1_psi_rp_rapidity_{};
  std::unique_ptr<TProfile> v1_psi_sp_rapidity_{};
  std::unique_ptr<TProfile> v1_psi_half_rapidity_{};
  std::unique_ptr<TProfile> v1_nonflow_true_{};
  std::unique_ptr<TProfile> v1_nonflow_estimated_{};
  
  std::unique_ptr<TH1F> h1_y_{};
  std::unique_ptr<TH1F> h1_pT_{};

  std::unique_ptr<TH1F> h1_Qx_{};
  std::unique_ptr<TH1F> h1_Qy_{};
  
  MeanCalculator res_psi_sp_{};
  MeanCalculator res_half_{};

  double sum_projectile_psi_{};
  double sum_half_psi_{};
  double sum_produced_psi_{};
  double sum_produced_projectile_{};
  size_t n_analyzed_{};
};

#endif // FLOW_ANALYZER_H