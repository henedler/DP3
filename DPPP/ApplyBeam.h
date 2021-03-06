// ApplyBeam.h: DPPP step class to ApplyBeam visibilities from a source model
// Copyright (C) 2013
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

/// @file
/// @brief DPPP step class to apply the beam model (optionally inverted)
/// @author Tammo Jan Dijkema

#ifndef DPPP_APPLYBEAM_H
#define DPPP_APPLYBEAM_H

#include "DPInput.h"
#include "DPBuffer.h"
#include "Position.h"

#include <EveryBeam/station.h>
#include <EveryBeam/common/types.h>

#include <casacore/casa/Arrays/Cube.h>
#include <casacore/measures/Measures/MDirection.h>

namespace DP3 {

class ParameterSet;

namespace DPPP {

/// \brief DPPP step class to ApplyBeam visibilities from a source model

/// This class is a DPStep class to apply the beam model, optionally inverted.
/// The input MeasurementSet it operates on, must have the LOFAR subtables
/// defining the station layout and tiles/dipoles used.

class ApplyBeam : public DPStep {
 public:
  /// Construct the object.
  /// Parameters are obtained from the parset using the given prefix.
  ApplyBeam(DPInput*, const ParameterSet&, const string& prefix,
            bool substep = false);

  ApplyBeam();

  virtual ~ApplyBeam();

  /// Process the data.
  /// It keeps the data.
  /// When processed, it invokes the process function of the next step.
  virtual bool process(const DPBuffer& buffer) {
    return processMultithreaded(buffer, 0);
  }

  /// If apply beam is called from multiple threads, it needs the thread index
  /// to determine what scratch space to use etc.
  bool processMultithreaded(const DPBuffer&, size_t thread);

  /// Finish the processing of this step and subsequent steps.
  virtual void finish();

  /// Update the general info.
  virtual void updateInfo(const DPInfo&);

  /// Show the step parameters.
  virtual void show(std::ostream&) const;

  /// Show the timings.
  virtual void showTimings(std::ostream&, double duration) const;

  bool invert() { return itsInvert; }

  template <typename T>
  static void applyBeam(const DPInfo& info, double time, T* data0,
                        float* weight0, const everybeam::vector3r_t& srcdir,
                        const everybeam::vector3r_t& refdir,
                        const everybeam::vector3r_t& tiledir,
                        const std::vector<everybeam::Station::Ptr>& antBeamInfo,
                        std::vector<everybeam::matrix22c_t>& beamValues,
                        bool useChannelFreq, bool invert,
                        BeamCorrectionMode mode,
                        everybeam::ElementResponseModel element_reponse_model,
                        bool doUpdateWeights = false);

  template <typename T>
  static void applyBeamStokesIArrayFactor(
      const DPInfo& info, double time, T* data0, float* weight0,
      const everybeam::vector3r_t& srcdir, const everybeam::vector3r_t& refdir,
      const everybeam::vector3r_t& tiledir,
      const std::vector<everybeam::Station::Ptr>& antBeamInfo,
      std::vector<everybeam::complex_t>& beamValues, bool useChannelFreq,
      bool invert, BeamCorrectionMode mode,
      everybeam::ElementResponseModel element_reponse_model,
      bool doUpdateWeights = false);

 private:
  everybeam::vector3r_t dir2Itrf(const casacore::MDirection& dir,
                                 casacore::MDirection::Convert& measConverter);

  DPInput* itsInput;
  string itsName;
  DPBuffer itsBuffer;
  bool itsInvert;
  bool itsUpdateWeights;
  std::vector<string> itsDirectionStr;
  casacore::MDirection itsDirection;
  bool itsUseChannelFreq;
  BeamCorrectionMode itsMode;
  everybeam::ElementResponseModel itsElementResponseModel;

  /// If a beam had already been applied before running this step, that beam
  /// needs to undone; hence we register that beam info here:
  ///@{
  casacore::MDirection itsDirectionAtStart;
  BeamCorrectionMode itsModeAtStart;
  ///@}

  unsigned int itsDebugLevel;

  /// The info needed to calculate the station beams.
  ///@{
  std::vector<std::vector<everybeam::Station::Ptr> > itsAntBeamInfo;
  std::vector<casacore::MeasFrame> itsMeasFrames;
  std::vector<casacore::MDirection::Convert> itsMeasConverters;
  std::vector<std::vector<everybeam::matrix22c_t> > itsBeamValues;
  ///@}

  NSTimer itsTimer;
};

}  // namespace DPPP
}  // namespace DP3

#endif  // HAVE_LOFAR_BEAM
