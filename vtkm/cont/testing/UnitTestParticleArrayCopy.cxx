//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include <random>
#include <vtkm/cont/ParticleArrayCopy.h>
#include <vtkm/cont/testing/Testing.h>

void TestParticleArrayCopy()
{
  std::random_device device;
  std::default_random_engine generator(static_cast<vtkm::UInt32>(277));
  vtkm::FloatDefault x0(-1), x1(1);
  std::uniform_real_distribution<vtkm::FloatDefault> dist(x0, x1);

  std::vector<vtkm::Particle> particles;
  vtkm::Id N = 17;
  for (vtkm::Id i = 0; i < N; i++)
  {
    auto x = dist(generator);
    auto y = dist(generator);
    auto z = dist(generator);
    particles.push_back(vtkm::Particle(vtkm::Vec3f(x, y, z), i));
  }

  for (int i = 0; i < 2; i++)
  {
    auto particleAH = vtkm::cont::make_ArrayHandle(particles);

    //Test copy position only
    if (i == 0)
    {
      vtkm::cont::ArrayHandle<vtkm::Vec3f> pos;
      vtkm::cont::ParticleArrayCopy(particleAH, pos);

      auto pPortal = particleAH.ReadPortal();
      for (vtkm::Id j = 0; j < N; j++)
      {
        auto p = pPortal.Get(j);
        auto pt = pos.ReadPortal().Get(j);
        VTKM_TEST_ASSERT(p.Pos == pt, "Positions do not match");
      }
    }
    else //Test copy everything
    {
      vtkm::cont::ArrayHandle<vtkm::Vec3f> pos;
      vtkm::cont::ArrayHandle<vtkm::Id> ids, steps;
      vtkm::cont::ArrayHandle<vtkm::ParticleStatus> status;
      vtkm::cont::ArrayHandle<vtkm::FloatDefault> ptime;

      vtkm::cont::ParticleArrayCopy(particleAH, pos, ids, steps, status, ptime);

      auto pPortal = particleAH.ReadPortal();
      for (vtkm::Id j = 0; j < N; j++)
      {
        auto p = pPortal.Get(j);
        auto pt = pos.ReadPortal().Get(j);
        VTKM_TEST_ASSERT(p.Pos == pt, "Positions do not match");
        VTKM_TEST_ASSERT(p.ID == ids.ReadPortal().Get(j), "IDs do not match");
        VTKM_TEST_ASSERT(p.NumSteps == steps.ReadPortal().Get(j), "Steps do not match");
        VTKM_TEST_ASSERT(p.Status == status.ReadPortal().Get(j), "Steps do not match");
        VTKM_TEST_ASSERT(p.Time == ptime.ReadPortal().Get(j), "Times do not match");
      }
    }
  }
}

int UnitTestParticleArrayCopy(int argc, char* argv[])
{
  return vtkm::cont::testing::Testing::Run(TestParticleArrayCopy, argc, argv);
}
