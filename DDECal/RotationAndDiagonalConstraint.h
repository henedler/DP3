// Copyright (C) 2020
// ASTRON (Netherlands Institute for Radio Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//
// This file is part of the LOFAR software suite.
// The LOFAR software suite is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The LOFAR software suite is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

#ifndef ROTATIONANDDIAGONAL_CONSTRAINT_H
#define ROTATIONANDDIAGONAL_CONSTRAINT_H

#include "Constraint.h"

#include <vector>
#include <ostream>

namespace DP3 {

class RotationAndDiagonalConstraint : public Constraint {
 public:
  RotationAndDiagonalConstraint();

  virtual std::vector<Result> Apply(
      std::vector<std::vector<dcomplex> >& solutions, double time,
      std::ostream* statStream);

  virtual void InitializeDimensions(size_t nAntennas, size_t nDirections,
                                    size_t nChannelBlocks);

  virtual void SetWeights(const std::vector<double>& weights);

  void SetDoRotationReference(bool doRotationReference);

 private:
  std::vector<Constraint::Result> _res;
  bool _doRotationReference;
};

}  // namespace DP3

#endif
