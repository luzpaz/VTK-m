//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2016 Sandia Corporation.
//  Copyright 2016 UT-Battelle, LLC.
//  Copyright 2016 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/rendering/View3D.h>

namespace vtkm
{
namespace rendering
{

View3D::View3D(const vtkm::rendering::Scene& scene,
               const vtkm::rendering::Mapper& mapper,
               const vtkm::rendering::Canvas& canvas,
               const vtkm::rendering::Color& backgroundColor)
  : View(scene, mapper, canvas, backgroundColor)
{
}

View3D::View3D(const vtkm::rendering::Scene& scene,
               const vtkm::rendering::Mapper& mapper,
               const vtkm::rendering::Canvas& canvas,
               const vtkm::rendering::Camera& camera,
               const vtkm::rendering::Color& backgroundColor)
  : View(scene, mapper, canvas, camera, backgroundColor)
{
}

View3D::~View3D()
{
}

void View3D::Paint()
{
  this->GetCanvas().Activate();
  this->GetCanvas().Clear();
  this->SetupForWorldSpace();
  this->GetScene().Render(this->GetMapper(), this->GetCanvas(), this->GetCamera());
  this->RenderWorldAnnotations();

  this->SetupForScreenSpace();
  this->RenderScreenAnnotations();

  this->GetCanvas().Finish();
}

void View3D::RenderScreenAnnotations()
{
  if (this->GetScene().GetNumberOfActors() > 0)
  {
    //this->ColorBarAnnotation.SetAxisColor(vtkm::rendering::Color(1,1,1));
    this->ColorBarAnnotation.SetRange(this->GetScene().GetActor(0).GetScalarRange(), 5);
    this->ColorBarAnnotation.SetColorTable(this->GetScene().GetActor(0).GetColorTable());
    this->ColorBarAnnotation.Render(
      this->GetCamera(), this->GetWorldAnnotator(), this->GetCanvas());
  }
}

void View3D::RenderWorldAnnotations()
{
  vtkm::Bounds bounds = this->GetScene().GetSpatialBounds();
  vtkm::Float64 xmin = bounds.X.Min, xmax = bounds.X.Max;
  vtkm::Float64 ymin = bounds.Y.Min, ymax = bounds.Y.Max;
  vtkm::Float64 zmin = bounds.Z.Min, zmax = bounds.Z.Max;
  vtkm::Float64 dx = xmax - xmin, dy = ymax - ymin, dz = zmax - zmin;
  vtkm::Float64 size = vtkm::Sqrt(dx * dx + dy * dy + dz * dz);

  this->BoxAnnotation.SetColor(Color(.5f, .5f, .5f));
  this->BoxAnnotation.SetExtents(this->GetScene().GetSpatialBounds());
  this->BoxAnnotation.Render(this->GetCamera(), this->GetWorldAnnotator());

  vtkm::Vec<vtkm::Float32, 3> lookAt = this->GetCamera().GetLookAt();
  vtkm::Vec<vtkm::Float32, 3> position = this->GetCamera().GetPosition();
  bool xtest = lookAt[0] > position[0];
  bool ytest = lookAt[1] > position[1];
  bool ztest = lookAt[2] > position[2];

  const bool outsideedges = true; // if false, do closesttriad
  if (outsideedges)
  {
    xtest = !xtest;
    //ytest = !ytest;
  }

  vtkm::Float64 xrel = vtkm::Abs(dx) / size;
  vtkm::Float64 yrel = vtkm::Abs(dy) / size;
  vtkm::Float64 zrel = vtkm::Abs(dz) / size;

  this->XAxisAnnotation.SetAxis(0);
  this->XAxisAnnotation.SetColor(Color(1, 1, 1));
  this->XAxisAnnotation.SetTickInvert(xtest, ytest, ztest);
  this->XAxisAnnotation.SetWorldPosition(
    xmin, ytest ? ymin : ymax, ztest ? zmin : zmax, xmax, ytest ? ymin : ymax, ztest ? zmin : zmax);
  this->XAxisAnnotation.SetRange(xmin, xmax);
  this->XAxisAnnotation.SetMajorTickSize(size / 40.f, 0);
  this->XAxisAnnotation.SetMinorTickSize(size / 80.f, 0);
  this->XAxisAnnotation.SetLabelFontOffset(vtkm::Float32(size / 15.f));
  this->XAxisAnnotation.SetMoreOrLessTickAdjustment(xrel < .3 ? -1 : 0);
  this->XAxisAnnotation.Render(this->GetCamera(), this->GetWorldAnnotator(), this->GetCanvas());

  this->YAxisAnnotation.SetAxis(1);
  this->YAxisAnnotation.SetColor(Color(1, 1, 1));
  this->YAxisAnnotation.SetTickInvert(xtest, ytest, ztest);
  this->YAxisAnnotation.SetWorldPosition(
    xtest ? xmin : xmax, ymin, ztest ? zmin : zmax, xtest ? xmin : xmax, ymax, ztest ? zmin : zmax);
  this->YAxisAnnotation.SetRange(ymin, ymax);
  this->YAxisAnnotation.SetMajorTickSize(size / 40.f, 0);
  this->YAxisAnnotation.SetMinorTickSize(size / 80.f, 0);
  this->YAxisAnnotation.SetLabelFontOffset(vtkm::Float32(size / 15.f));
  this->YAxisAnnotation.SetMoreOrLessTickAdjustment(yrel < .3 ? -1 : 0);
  this->YAxisAnnotation.Render(this->GetCamera(), this->GetWorldAnnotator(), this->GetCanvas());

  this->ZAxisAnnotation.SetAxis(2);
  this->ZAxisAnnotation.SetColor(Color(1, 1, 1));
  this->ZAxisAnnotation.SetTickInvert(xtest, ytest, ztest);
  this->ZAxisAnnotation.SetWorldPosition(
    xtest ? xmin : xmax, ytest ? ymin : ymax, zmin, xtest ? xmin : xmax, ytest ? ymin : ymax, zmax);
  this->ZAxisAnnotation.SetRange(zmin, zmax);
  this->ZAxisAnnotation.SetMajorTickSize(size / 40.f, 0);
  this->ZAxisAnnotation.SetMinorTickSize(size / 80.f, 0);
  this->ZAxisAnnotation.SetLabelFontOffset(vtkm::Float32(size / 15.f));
  this->ZAxisAnnotation.SetMoreOrLessTickAdjustment(zrel < .3 ? -1 : 0);
  this->ZAxisAnnotation.Render(this->GetCamera(), this->GetWorldAnnotator(), this->GetCanvas());
}
}
} // namespace vtkm::rendering
