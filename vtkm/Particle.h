//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_m_Particle_h
#define vtk_m_Particle_h

#include <ostream>
#include <vtkm/Bitset.h>
#include <vtkm/VecVariable.h>
#include <vtkm/VectorAnalysis.h>
#include <vtkm/cont/Serialization.h>

namespace vtkm
{

//Bit field describing the status:
class ParticleStatus : public vtkm::Bitset<vtkm::UInt8>
{
public:
  VTKM_EXEC_CONT ParticleStatus()
  {
    this->SetOk();
    this->ClearTerminate();
  }

  VTKM_EXEC_CONT void SetOk() { this->set(this->SUCCESS_BIT); }
  VTKM_EXEC_CONT bool CheckOk() const { return this->test(this->SUCCESS_BIT); }

  VTKM_EXEC_CONT void SetFail() { this->reset(this->SUCCESS_BIT); }
  VTKM_EXEC_CONT bool CheckFail() const { return !this->test(this->SUCCESS_BIT); }

  VTKM_EXEC_CONT void SetTerminate() { this->set(this->TERMINATE_BIT); }
  VTKM_EXEC_CONT void ClearTerminate() { this->reset(this->TERMINATE_BIT); }
  VTKM_EXEC_CONT bool CheckTerminate() const { return this->test(this->TERMINATE_BIT); }

  VTKM_EXEC_CONT void SetSpatialBounds() { this->set(this->SPATIAL_BOUNDS_BIT); }
  VTKM_EXEC_CONT void ClearSpatialBounds() { this->reset(this->SPATIAL_BOUNDS_BIT); }
  VTKM_EXEC_CONT bool CheckSpatialBounds() const { return this->test(this->SPATIAL_BOUNDS_BIT); }

  VTKM_EXEC_CONT void SetTemporalBounds() { this->set(this->TEMPORAL_BOUNDS_BIT); }
  VTKM_EXEC_CONT void ClearTemporalBounds() { this->reset(this->TEMPORAL_BOUNDS_BIT); }
  VTKM_EXEC_CONT bool CheckTemporalBounds() const { return this->test(this->TEMPORAL_BOUNDS_BIT); }

  VTKM_EXEC_CONT void SetTookAnySteps() { this->set(this->TOOK_ANY_STEPS_BIT); }
  VTKM_EXEC_CONT void ClearTookAnySteps() { this->reset(this->TOOK_ANY_STEPS_BIT); }
  VTKM_EXEC_CONT bool CheckTookAnySteps() const { return this->test(this->TOOK_ANY_STEPS_BIT); }

  VTKM_EXEC_CONT void SetInGhostCell() { this->set(this->IN_GHOST_CELL_BIT); }
  VTKM_EXEC_CONT void ClearInGhostCell() { this->reset(this->IN_GHOST_CELL_BIT); }
  VTKM_EXEC_CONT bool CheckInGhostCell() const { return this->test(this->IN_GHOST_CELL_BIT); }

private:
  static constexpr vtkm::Id SUCCESS_BIT = 0;
  static constexpr vtkm::Id TERMINATE_BIT = 1;
  static constexpr vtkm::Id SPATIAL_BOUNDS_BIT = 2;
  static constexpr vtkm::Id TEMPORAL_BOUNDS_BIT = 3;
  static constexpr vtkm::Id TOOK_ANY_STEPS_BIT = 4;
  static constexpr vtkm::Id IN_GHOST_CELL_BIT = 5;
};

inline VTKM_CONT std::ostream& operator<<(std::ostream& s, const vtkm::ParticleStatus& status)
{
  s << "[ok= " << status.CheckOk();
  s << " term= " << status.CheckTerminate();
  s << " spat= " << status.CheckSpatialBounds();
  s << " temp= " << status.CheckTemporalBounds();
  s << " ghst= " << status.CheckInGhostCell();
  s << "]";
  return s;
}

class Particle
{
public:
  VTKM_EXEC_CONT
  Particle() {}

  VTKM_EXEC_CONT
  Particle(const vtkm::Vec3f& p,
           const vtkm::Id& id,
           const vtkm::Id& numSteps = 0,
           const vtkm::ParticleStatus& status = vtkm::ParticleStatus(),
           const vtkm::FloatDefault& time = 0)
    : Pos(p)
    , ID(id)
    , NumSteps(numSteps)
    , Status(status)
    , Time(time)
  {
  }

  VTKM_EXEC_CONT
  Particle(const vtkm::Particle& p)
    : Pos(p.Pos)
    , ID(p.ID)
    , NumSteps(p.NumSteps)
    , Status(p.Status)
    , Time(p.Time)
  {
  }

  vtkm::Particle& operator=(const vtkm::Particle&) = default;

  VTKM_EXEC_CONT ~Particle() noexcept
  {
    // This must not be defaulted, since defaulted virtual destructors are
    // troublesome with CUDA __host__ __device__ markup.
  }

  VTKM_EXEC_CONT
  vtkm::Vec3f Velocity(const vtkm::VecVariable<vtkm::Vec3f, 2>& vectors,
                       const vtkm::FloatDefault& vtkmNotUsed(length))
  {
    // Velocity is evaluated from the Velocity field
    // and is not influenced by the particle
    VTKM_ASSERT(vectors.GetNumberOfComponents() > 0);
    return vectors[0];
  }

  VTKM_EXEC_CONT
  vtkm::Vec3f GetEvaluationPosition(const vtkm::FloatDefault& deltaT) const
  {
    (void)deltaT; // unused for a general particle advection case
    return this->Pos;
  }

  inline VTKM_CONT friend std::ostream& operator<<(std::ostream& out, const vtkm::Particle& p)
  {
    out << "v(" << p.Time << ") = " << p.Pos << ", ID: " << p.ID << ", NumSteps: " << p.NumSteps
        << ", Status: " << p.Status;
    return out;
  }

  vtkm::Vec3f Pos;
  vtkm::Id ID = -1;
  vtkm::Id NumSteps = 0;
  vtkm::ParticleStatus Status;
  vtkm::FloatDefault Time = 0;

  static size_t Sizeof()
  {
    constexpr std::size_t sz = sizeof(vtkm::Vec3f) // Pos
      + sizeof(vtkm::Id)                           // ID
      + sizeof(vtkm::Id)                           // NumSteps
      + sizeof(vtkm::UInt8)                        // Status
      + sizeof(vtkm::FloatDefault);                // Time

    return sz;
  }
};

class ChargedParticle
{
public:
  VTKM_EXEC_CONT
  ChargedParticle() {}

  VTKM_EXEC_CONT
  ChargedParticle(const vtkm::Vec3f& position,
                  const vtkm::Id& id,
                  const vtkm::FloatDefault& mass,
                  const vtkm::FloatDefault& charge,
                  const vtkm::FloatDefault& weighting,
                  const vtkm::Vec3f& momentum,
                  const vtkm::Id& numSteps = 0,
                  const vtkm::ParticleStatus& status = vtkm::ParticleStatus(),
                  const vtkm::FloatDefault& time = 0)
    : Pos(position)
    , ID(id)
    , NumSteps(numSteps)
    , Status(status)
    , Time(time)
    , Mass(mass)
    , Charge(charge)
    , Weighting(weighting)
    , Momentum(momentum)
  {
  }

  VTKM_EXEC_CONT
  vtkm::FloatDefault Gamma(vtkm::Vec3f momentum, bool reciprocal = false) const
  {
    constexpr vtkm::FloatDefault c2 = SPEED_OF_LIGHT * SPEED_OF_LIGHT;
    const auto fMom2 = vtkm::MagnitudeSquared(momentum);
    const auto m2 = this->Mass * this->Mass;
    const auto m2_c2_reci = 1.0 / (m2 * c2);
    if (reciprocal)
      return static_cast<vtkm::FloatDefault>(vtkm::RSqrt(1.0 + fMom2 * m2_c2_reci));
    else
      return static_cast<vtkm::FloatDefault>(vtkm::Sqrt(1.0 + fMom2 * m2_c2_reci));
  }

  VTKM_EXEC_CONT
  vtkm::Vec3f Velocity(const vtkm::VecVariable<vtkm::Vec3f, 2>& vectors,
                       const vtkm::FloatDefault& length)
  {
    VTKM_ASSERT(vectors.GetNumberOfComponents() == 2);

    // Suppress unused warning
    (void)this->Weighting;

    vtkm::Vec3f eField = vectors[0];
    vtkm::Vec3f bField = vectors[1];

    const vtkm::FloatDefault QoM = this->Charge / this->Mass;
    const vtkm::Vec3f mom_minus = this->Momentum + (0.5 * this->Charge * eField * length);

    // Get reciprocal of Gamma
    vtkm::Vec3f gamma_reci = this->Gamma(mom_minus, true);
    const vtkm::Vec3f t = 0.5 * QoM * length * bField * gamma_reci;
    const vtkm::Vec3f s = 2.0f * t * (1.0 / (1.0 + vtkm::Magnitude(t)));
    const vtkm::Vec3f mom_prime = mom_minus + vtkm::Cross(mom_minus, t);
    const vtkm::Vec3f mom_plus = mom_minus + vtkm::Cross(mom_prime, s);

    const vtkm::Vec3f mom_new = mom_plus + 0.5 * this->Charge * eField * length;
    //TODO : Sould this be a const method?
    // If yes, need a better way to update momentum
    this->Momentum = mom_new;

    // momentum = velocity * mass * gamma;
    // --> velocity = momentum / (mass * gamma)
    // --> velocity = ( momentum / mass ) * gamma_reci
    vtkm::Vec3f velocity = (mom_new / this->Mass) * this->Gamma(mom_new, true);
    return velocity;
  }

  VTKM_EXEC_CONT
  vtkm::Vec3f GetEvaluationPosition(const vtkm::FloatDefault& deltaT) const
  {
    // Translation is in -ve Z direction,
    // this needs to be a parameter.
    auto translation = this->NumSteps * deltaT * SPEED_OF_LIGHT * vtkm::Vec3f{ 0., 0., -1.0 };
    return this->Pos + translation;
  }

  inline VTKM_CONT friend std::ostream& operator<<(std::ostream& out,
                                                   const vtkm::ChargedParticle& p)
  {
    out << "v(" << p.Time << ") = " << p.Pos << ", ID: " << p.ID << ", NumSteps: " << p.NumSteps
        << ", Status: " << p.Status;
    return out;
  }

  vtkm::Vec3f Pos;
  vtkm::Id ID = -1;
  vtkm::Id NumSteps = 0;
  vtkm::ParticleStatus Status;
  vtkm::FloatDefault Time = 0;

private:
  vtkm::FloatDefault Mass;
  vtkm::FloatDefault Charge;
  vtkm::FloatDefault Weighting;
  vtkm::Vec3f Momentum;
  constexpr static vtkm::FloatDefault SPEED_OF_LIGHT =
    static_cast<vtkm::FloatDefault>(2.99792458e8);

  friend struct mangled_diy_namespace::Serialization<vtkm::ChargedParticle>;

public:
  static size_t Sizeof()
  {
    constexpr std::size_t sz = sizeof(vtkm::Vec3f) // Pos
      + sizeof(vtkm::Id)                           // ID
      + sizeof(vtkm::Id)                           // NumSteps
      + sizeof(vtkm::UInt8)                        // Status
      + sizeof(vtkm::FloatDefault)                 // Time
      + sizeof(vtkm::FloatDefault)                 //Mass
      + sizeof(vtkm::FloatDefault)                 //Charge
      + sizeof(vtkm::FloatDefault)                 //Weighting
      + sizeof(vtkm::Vec3f)                        //Momentum
      + sizeof(vtkm::FloatDefault);                //Speed_of_light

    return sz;
  }
};

} //namespace vtkm


namespace mangled_diy_namespace
{
template <>
struct Serialization<vtkm::Particle>
{
public:
  static VTKM_CONT void save(BinaryBuffer& bb, const vtkm::Particle& p)
  {
    vtkmdiy::save(bb, p.Pos);
    vtkmdiy::save(bb, p.ID);
    vtkmdiy::save(bb, p.NumSteps);
    vtkmdiy::save(bb, p.Status);
    vtkmdiy::save(bb, p.Time);
  }

  static VTKM_CONT void load(BinaryBuffer& bb, vtkm::Particle& p)
  {
    vtkmdiy::load(bb, p.Pos);
    vtkmdiy::load(bb, p.ID);
    vtkmdiy::load(bb, p.NumSteps);
    vtkmdiy::load(bb, p.Status);
    vtkmdiy::load(bb, p.Time);
  }
};

template <>
struct Serialization<vtkm::ChargedParticle>
{
public:
  static VTKM_CONT void save(BinaryBuffer& bb, const vtkm::ChargedParticle& e)
  {
    vtkmdiy::save(bb, e.Pos);
    vtkmdiy::save(bb, e.ID);
    vtkmdiy::save(bb, e.NumSteps);
    vtkmdiy::save(bb, e.Status);
    vtkmdiy::save(bb, e.Time);
    vtkmdiy::save(bb, e.Mass);
    vtkmdiy::save(bb, e.Charge);
    vtkmdiy::save(bb, e.Weighting);
    vtkmdiy::save(bb, e.Momentum);
  }

  static VTKM_CONT void load(BinaryBuffer& bb, vtkm::ChargedParticle& e)
  {
    vtkmdiy::load(bb, e.Pos);
    vtkmdiy::load(bb, e.ID);
    vtkmdiy::load(bb, e.NumSteps);
    vtkmdiy::load(bb, e.Status);
    vtkmdiy::load(bb, e.Time);
    vtkmdiy::load(bb, e.Mass);
    vtkmdiy::load(bb, e.Charge);
    vtkmdiy::load(bb, e.Weighting);
    vtkmdiy::load(bb, e.Momentum);
  }
};
}

#endif // vtk_m_Particle_h
