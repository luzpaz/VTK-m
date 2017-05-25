//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#ifndef vtk_m_filter_Threshold_h
#define vtk_m_filter_Threshold_h

#include <vtkm/filter/FilterDataSetWithField.h>
#include <vtkm/worklet/Threshold.h>

namespace vtkm
{
namespace filter
{

class Threshold : public vtkm::filter::FilterDataSetWithField<Threshold>
{
public:
  VTKM_CONT
  Threshold();

  VTKM_CONT
  void SetLowerThreshold(vtkm::Float64 value) { this->LowerValue = value; }
  VTKM_CONT
  void SetUpperThreshold(vtkm::Float64 value) { this->UpperValue = value; }

  VTKM_CONT
  vtkm::Float64 GetLowerThreshold() const { return this->LowerValue; }
  VTKM_CONT
  vtkm::Float64 GetUpperThreshold() const { return this->UpperValue; }

  template <typename T, typename StorageType, typename DerivedPolicy, typename DeviceAdapter>
  VTKM_CONT vtkm::filter::ResultDataSet DoExecute(
    const vtkm::cont::DataSet& input, const vtkm::cont::ArrayHandle<T, StorageType>& field,
    const vtkm::filter::FieldMetadata& fieldMeta,
    const vtkm::filter::PolicyBase<DerivedPolicy>& policy, const DeviceAdapter& tag);

  //Map a new field onto the resulting dataset after running the filter
  //this call is only valid
  template <typename T, typename StorageType, typename DerivedPolicy, typename DeviceAdapter>
  VTKM_CONT bool DoMapField(vtkm::filter::ResultDataSet& result,
                            const vtkm::cont::ArrayHandle<T, StorageType>& input,
                            const vtkm::filter::FieldMetadata& fieldMeta,
                            const vtkm::filter::PolicyBase<DerivedPolicy>& policy,
                            const DeviceAdapter& tag);

private:
  double LowerValue;
  double UpperValue;
  vtkm::cont::ArrayHandle<vtkm::Id> ValidCellIds;
};

template <>
class FilterTraits<Threshold>
{ //currently the threshold filter only works on scalar data.
public:
  typedef TypeListTagScalarAll InputFieldTypeList;
};
}
} // namespace vtkm::filter

#include <vtkm/filter/Threshold.hxx>

#endif // vtk_m_filter_Threshold_h
