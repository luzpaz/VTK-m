//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#ifndef vtk_m_worklet_particleadvection_Particles_h
#define vtk_m_worklet_particleadvection_Particles_h

class ParticleExecutionObjectType;

#include <vtkm/Types.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/ExecutionObjectBase.h>

namespace vtkm
{
namespace worklet
{
namespace particleadvection
{

using ScalarType = vtkm::FloatDefault;

enum class ParticleStatus
{
  OK = 1,
  TERMINATED = 1 << 1,
  EXIT_SPATIAL_BOUNDARY = 1 << 2,
  EXIT_TEMPORAL_BOUNDARY = 1 << 3,
  ERROR = 1 << 4,
  TOOK_ANY_STEPS = 1 << 5
};

template <typename Device>
class ParticleExecutionObject
{
public:
  using ScalarType = vtkm::worklet::particleadvection::ScalarType;
  using VectorType = vtkm::Vec<ScalarType, 3>;

  VTKM_EXEC_CONT
  ParticleExecutionObject()
    : Pos()
    , Status()
    , Steps()
    , Time()
    , MaxSteps(0)
  {
  }

  ParticleExecutionObject(vtkm::cont::ArrayHandle<VectorType> posArray,
                          vtkm::cont::ArrayHandle<vtkm::Id> stepsArray,
                          vtkm::cont::ArrayHandle<vtkm::Id> statusArray,
                          vtkm::cont::ArrayHandle<ScalarType> timeArray,
                          vtkm::Id maxSteps)
  {
    Pos = posArray.PrepareForInPlace(Device());
    Steps = stepsArray.PrepareForInPlace(Device());
    Status = statusArray.PrepareForInPlace(Device());
    Time = timeArray.PrepareForInPlace(Device());
    MaxSteps = maxSteps;
  }

  VTKM_EXEC
  void TakeStep(const vtkm::Id& idx, const VectorType& pt)
  {
    // Irrespective of what the advected status of the particle is,
    // we need to set the output position as the last step taken by
    // the particle, and increase the number of steps take by 1.
    this->Pos.Set(idx, pt);
    vtkm::Id nSteps = Steps.Get(idx);
    this->Steps.Set(idx, ++nSteps);

    // Check if the particle has completed the maximum steps required.
    // If yes, set it to terminated.
    if (nSteps == MaxSteps)
      SetTerminated(idx);
  }

  /* Set/Change Status */
  VTKM_EXEC
  void SetOK(const vtkm::Id& idx)
  {
    Clear(idx);
    Status.Set(idx, ParticleStatus::OK);
  }
  VTKM_EXEC
  void SetTerminated(const vtkm::Id& idx)
  {
    ClearBit(idx, ParticleStatus::OK);
    SetBit(idx, ParticleStatus::TERMINATED);
  }
  VTKM_EXEC
  void SetTookAnySteps(const vtkm::Id& idx, const bool& val)
  {
    if (val)
      SetBit(idx, ParticleStatus::TOOK_ANY_STEPS);
    else
      ClearBit(idx, ParticleStatus::TOOK_ANY_STEPS);
  }
  VTKM_EXEC
  void SetExitSpatialBoundary(const vtkm::Id& idx)
  {
    ClearBit(idx, ParticleStatus::OK);
    SetBit(idx, ParticleStatus::EXIT_SPATIAL_BOUNDARY);
  }
  VTKM_EXEC
  void SetExitTemporalBoundary(const vtkm::Id& idx)
  {
    ClearBit(idx, ParticleStatus::OK);
    SetBit(idx, ParticleStatus::EXIT_TEMPORAL_BOUNDARY);
  }
  VTKM_EXEC
  void SetError(const vtkm::Id& idx)
  {
    ClearBit(idx, ParticleStatus::OK);
    SetBit(idx, ParticleStatus::ERROR);
  }

  /* Check Status */
  VTKM_EXEC
  bool OK(const vtkm::Id& idx) { return CheckBit(idx, ParticleStatus::OK); }
  VTKM_EXEC
  bool Terminated(const vtkm::Id& idx) { return CheckBit(idx, ParticleStatus::TERMINATED); }
  VTKM_EXEC
  bool ExitSpatialBoundary(const vtkm::Id& idx)
  {
    return CheckBit(idx, ParticleStatus::EXIT_SPATIAL_BOUNDARY);
  }
  VTKM_EXEC
  bool ExitTemporalBoundary(const vtkm::Id& idx)
  {
    return CheckBit(idx, ParticleStatus::EXIT_TEMPORAL_BOUNDARY);
  }
  VTKM_EXEC
  bool Error(const vtkm::Id& idx) { return CheckBit(idx, ParticleStatus::ERROR); }
  VTKM_EXEC
  bool Integrateable(const vtkm::Id& idx)
  {
    return OK(idx) && !(Terminated(idx) || ExitSpatialBoundary(idx) || ExitTemporalBoundary(idx));
  }
  VTKM_EXEC
  bool Done(const vtkm::Id& idx) { return !Integrateable(idx); }

  /* Bit Operations */
  VTKM_EXEC
  void Clear(const vtkm::Id& idx) { Status.Set(idx, 0); }
  VTKM_EXEC
  void SetBit(const vtkm::Id& idx, const ParticleStatus& b)
  {
    Status.Set(idx, Status.Get(idx) | static_cast<vtkm::Id>(b));
  }
  VTKM_EXEC
  void ClearBit(const vtkm::Id& idx, const ParticleStatus& b)
  {
    Status.Set(idx, Status.Get(idx) & ~static_cast<vtkm::Id>(b));
  }
  VTKM_EXEC
  bool CheckBit(const vtkm::Id& idx, const ParticleStatus& b) const
  {
    return (Status.Get(idx) & static_cast<vtkm::Id>(b)) != 0;
  }

  VTKM_EXEC
  VectorType GetPos(const vtkm::Id& idx) const { return Pos.Get(idx); }
  VTKM_EXEC
  vtkm::Id GetStep(const vtkm::Id& idx) const { return Steps.Get(idx); }
  VTKM_EXEC
  vtkm::Id GetStatus(const vtkm::Id& idx) const { return Status.Get(idx); }
  VTKM_EXEC
  ScalarType GetTime(const vtkm::Id& idx) const { return Time.Get(idx); }
  VTKM_EXEC
  void SetTime(const vtkm::Id& idx, ScalarType time) const { Time.Set(idx, time); }

protected:
  using IdPortal =
    typename vtkm::cont::ArrayHandle<vtkm::Id>::template ExecutionTypes<Device>::Portal;
  using PositionPortal =
    typename vtkm::cont::ArrayHandle<VectorType>::template ExecutionTypes<Device>::Portal;
  using FloatPortal =
    typename vtkm::cont::ArrayHandle<ScalarType>::template ExecutionTypes<Device>::Portal;

  PositionPortal Pos;
  IdPortal Status;
  IdPortal Steps;
  FloatPortal Time;
  vtkm::Id MaxSteps;
};


class Particles : public vtkm::cont::ExecutionObjectBase
{
private:
  using ScalarType = vtkm::worklet::particleadvection::ScalarType;
  using VectorType = vtkm::Vec<ScalarType, 3>;

public:
  template <typename Device>
  VTKM_CONT vtkm::worklet::particleadvection::ParticleExecutionObject<Device> PrepareForExecution(
    Device) const
  {

    return vtkm::worklet::particleadvection::ParticleExecutionObject<Device>(
      this->PosArray, this->StepsArray, this->StatusArray, this->TimeArray, this->MaxSteps);
  }

  VTKM_CONT
  Particles(vtkm::cont::ArrayHandle<VectorType>& posArray,
            vtkm::cont::ArrayHandle<vtkm::Id>& stepsArray,
            vtkm::cont::ArrayHandle<vtkm::Id>& statusArray,
            vtkm::cont::ArrayHandle<ScalarType>& timeArray,
            const vtkm::Id& maxSteps)
    : PosArray(posArray)
    , StepsArray(stepsArray)
    , StatusArray(statusArray)
    , TimeArray(timeArray)
    , MaxSteps(maxSteps)
  {
  }

  Particles() {}

protected:
  bool fromArray = false;

protected:
  vtkm::cont::ArrayHandle<VectorType> PosArray;
  vtkm::cont::ArrayHandle<vtkm::Id> StepsArray;
  vtkm::cont::ArrayHandle<vtkm::Id> StatusArray;
  vtkm::cont::ArrayHandle<ScalarType> TimeArray;
  vtkm::Id MaxSteps;
};


template <typename Device>
class StateRecordingParticleExecutionObject : public ParticleExecutionObject<Device>
{
  using ScalarType = vtkm::worklet::particleadvection::ScalarType;
  using VectorType = vtkm::Vec<ScalarType, 3>;

public:
  VTKM_EXEC_CONT
  StateRecordingParticleExecutionObject()
    : ParticleExecutionObject<Device>()
    , History()
    , Length(0)
    , ValidPoint()
  {
  }

  StateRecordingParticleExecutionObject(vtkm::cont::ArrayHandle<VectorType> posArray,
                                        vtkm::cont::ArrayHandle<VectorType> historyArray,
                                        vtkm::cont::ArrayHandle<vtkm::Id> stepsArray,
                                        vtkm::cont::ArrayHandle<vtkm::Id> statusArray,
                                        vtkm::cont::ArrayHandle<ScalarType> timeArray,
                                        vtkm::cont::ArrayHandle<vtkm::Id> validPointArray,
                                        vtkm::Id maxSteps)
    : ParticleExecutionObject<Device>(posArray, stepsArray, statusArray, timeArray, maxSteps)
  {
    Length = maxSteps;
    vtkm::Id numPos = posArray.GetNumberOfValues();
    History = historyArray.PrepareForOutput(numPos * Length, Device());
    ValidPoint = validPointArray.PrepareForInPlace(Device());
  }

  VTKM_EXEC_CONT
  void TakeStep(const vtkm::Id& idx, const VectorType& pt)
  {
    this->ParticleExecutionObject<Device>::TakeStep(idx, pt);

    //TakeStep incremented the step, so we want the PREV step value.
    vtkm::Id nSteps = this->Steps.Get(idx) - 1;

    // Update the step for streamline storing portals.
    // This includes updating the history and the valid points.
    vtkm::Id loc = idx * Length + nSteps;
    this->History.Set(loc, pt);
    this->ValidPoint.Set(loc, 1);
  }

  VectorType GetHistory(const vtkm::Id& idx, const vtkm::Id& step) const
  {
    return this->History.Get(idx * this->Length + step);
  }

protected:
  using IdPortal =
    typename vtkm::cont::ArrayHandle<vtkm::Id>::template ExecutionTypes<Device>::Portal;
  using PositionPortal =
    typename vtkm::cont::ArrayHandle<VectorType>::template ExecutionTypes<Device>::Portal;

  PositionPortal History;
  vtkm::Id Length;
  IdPortal ValidPoint;
};

class StateRecordingParticles : vtkm::cont::ExecutionObjectBase
{
public:
  using ScalarType = vtkm::worklet::particleadvection::ScalarType;
  using VectorType = vtkm::Vec<ScalarType, 3>;

  template <typename Device>
  VTKM_CONT vtkm::worklet::particleadvection::StateRecordingParticleExecutionObject<Device>
    PrepareForExecution(Device) const
  {
    return vtkm::worklet::particleadvection::StateRecordingParticleExecutionObject<Device>(
      PosArray, HistoryArray, StepsArray, StatusArray, TimeArray, ValidPointArray, MaxSteps);
  }

  VTKM_CONT
  StateRecordingParticles(vtkm::cont::ArrayHandle<VectorType>& posArray,
                          vtkm::cont::ArrayHandle<VectorType>& historyArray,
                          vtkm::cont::ArrayHandle<vtkm::Id>& stepsArray,
                          vtkm::cont::ArrayHandle<vtkm::Id>& statusArray,
                          vtkm::cont::ArrayHandle<ScalarType>& timeArray,
                          vtkm::cont::ArrayHandle<vtkm::Id>& validPointArray,
                          const vtkm::Id& maxSteps)
  {
    PosArray = posArray;
    HistoryArray = historyArray;
    StepsArray = stepsArray;
    StatusArray = statusArray;
    TimeArray = timeArray;
    ValidPointArray = validPointArray;
    MaxSteps = maxSteps;
  }

protected:
  vtkm::cont::ArrayHandle<vtkm::Id> StepsArray;
  vtkm::cont::ArrayHandle<vtkm::Id> StatusArray;
  vtkm::cont::ArrayHandle<ScalarType> TimeArray;
  vtkm::cont::ArrayHandle<vtkm::Id> ValidPointArray;
  vtkm::cont::ArrayHandle<VectorType> HistoryArray;
  vtkm::cont::ArrayHandle<VectorType> PosArray;
  vtkm::Id MaxSteps;
};


} //namespace particleadvection
} //namespace worklet
} //namespace vtkm

#endif // vtk_m_worklet_particleadvection_Particles_h
//============================================================================
