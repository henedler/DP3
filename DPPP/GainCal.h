// GainCal.h: DPPP step class to calibrate (direction independent) gains
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
/// @brief DPPP step class to apply a calibration correction to the data
/// @author Tammo Jan Dijkema

#ifndef DPPP_GAINCAL_H
#define DPPP_GAINCAL_H

#include "DPInput.h"
#include "DPBuffer.h"
#include "PhaseFitter.h"
#include "BaselineSelection.h"
#include "GainCalAlgorithm.h"
#include "Patch.h"
#include "UVWFlagger.h"
#include "Predict.h"
#include "SourceDBUtil.h"
#include "ApplyBeam.h"

#include "../ParmDB/Parm.h"
#include "../ParmDB/ParmFacade.h"
#include "../ParmDB/ParmSet.h"

#include <aocommon/parallelfor.h>
#include <aocommon/threadpool.h>

#include <EveryBeam/station.h>
#include <EveryBeam/common/types.h>

#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/Quanta/MVEpoch.h>
#include <casacore/measures/Measures/MEpoch.h>
#include <casacore/casa/Arrays/ArrayMath.h>

/// Convince HDF5 to use new API, even when system is configured to use 1.6 API
#define H5Acreate_vers 2
#define H5Tarray_create_vers 2
#define H5Dcreate_vers 2
#define H5Gcreate_vers 2
#include <H5Cpp.h>

namespace DP3 {

class ParameterSet;

namespace DPPP {

typedef std::vector<Patch::ConstPtr> PatchList;
typedef std::pair<size_t, size_t> Baseline;

/// @brief This class is a DPStep class to calibrate (direction independent)
/// gains.
class GainCal : public DPStep {
 public:
  enum CalType {
    DIAGONAL,
    SCALARCOMPLEXGAIN,
    FULLJONES,
    PHASEONLY,
    SCALARPHASE,
    AMPLITUDEONLY,
    SCALARAMPLITUDE,
    TECANDPHASE,
    TEC,
    TECSCREEN,
    ROTATIONANDDIAGONAL,
    ROTATION
  };

  /// Construct the object.
  /// Parameters are obtained from the parset using the given prefix.
  GainCal(DPInput*, const ParameterSet&, const std::string& prefix);

  virtual ~GainCal();

  /// Process the data.
  /// It keeps the data.
  /// When processed, it invokes the process function of the next step.
  virtual bool process(const DPBuffer&);

  /// Finish the processing of this step and subsequent steps.
  virtual void finish();

  /// Update the general info.
  virtual void updateInfo(const DPInfo&);

  /// Show the step parameters.
  virtual void show(std::ostream&) const;

  /// Show the timings.
  virtual void showTimings(std::ostream&, double duration) const;

  /// Convert string to a CalType
  static CalType stringToCalType(const std::string& mode);

  /// Convert CalType to a string
  static std::string calTypeToString(CalType caltype);

  /// Make a soltab with the given type
  static std::vector<H5Parm::SolTab> makeSolTab(
      H5Parm& h5parm, CalType caltype, std::vector<H5Parm::AxisInfo>& axes);

 private:
  /// Perform gaincal (polarized or unpolarized)
  void calibrate();

  /// Check for scalar mode
  static bool scalarMode(CalType caltype);

  /// Check for diagonal mode
  static bool diagonalMode(CalType caltype);

  /// Apply the solution
  void applySolution(DPBuffer& buf,
                     const casacore::Cube<casacore::DComplex>& invsol);

  /// Invert solution (for applying it)
  casacore::Cube<casacore::DComplex> invertSol(
      const casacore::Cube<casacore::DComplex>& sol);

  /// Fills the matrices itsVis and itsMVis
  void fillMatrices(casacore::Complex* model, casacore::Complex* data,
                    float* weight, const casacore::Bool* flag);

  /// Initialize the parmdb
  void initParmDB();

  /// Get parmdbname from itsMode
  std::string parmName();

  /// Determine which stations are used
  void setAntennaUsed();

  /// Write out the solutions of the current parameter chunk
  /// (timeslotsperparmupdate) Variant for writing ParmDB
  void writeSolutionsParmDB(double startTime);

  /// Write out the solutions of the current parameter chunk
  /// (timeslotsperparmupdate) Variant for writing H5Parm
  void writeSolutionsH5Parm(double startTime);

  DPInput* itsInput;
  std::string itsName;
  std::vector<DPBuffer> itsBuf;
  bool itsUseModelColumn;
  casacore::Cube<casacore::Complex> itsModelData;
  std::string itsParmDBName;
  bool itsUseH5Parm;
  std::shared_ptr<BBS::ParmDB> itsParmDB;
  std::string itsParsetString;  ///< Parset, for logging in H5Parm

  CalType itsMode;

  unsigned int itsDebugLevel;
  bool itsDetectStalling;

  bool itsApplySolution;

  std::vector<casacore::Cube<casacore::DComplex> >
      itsSols;  ///< for every timeslot, nCr x nSt x nFreqCells
  std::vector<casacore::Matrix<double> >
      itsTECSols;  ///< for every timeslot, 2 x nSt (alpha and beta)
  std::vector<double> itsFreqData;  ///< Mean frequency for every freqcell

  std::vector<std::unique_ptr<PhaseFitter> > itsPhaseFitters;  ///< Length nSt

  std::vector<GainCalAlgorithm> iS;

  UVWFlagger itsUVWFlagStep;
  ResultStep::ShPtr
      itsDataResultStep;  ///< Result step for data after UV-flagging

  std::unique_ptr<Predict> itsPredictStep;
  aocommon::ParallelFor<size_t> itsParallelFor;
  std::unique_ptr<class aocommon::ThreadPool> itsThreadPool;
  std::mutex itsMeasuresMutex;
  ApplyBeam itsApplyBeamStep;  ///< Beam step for applying beam to modelcol
  ResultStep::ShPtr
      itsResultStep;  ///< For catching results from Predict or Beam
  bool itsApplyBeamToModelColumn;

  BaselineSelection itsBaselineSelection;  ///< Filter
  casacore::Vector<bool> itsSelectedBL;   ///< Vector (length nBl) telling which
                                          ///< baselines are selected
  casacore::Vector<bool> itsAntennaUsed;  ///< Vector (length nSt) telling which
                                          ///< stations are solved for

  std::map<std::string, int> itsParmIdMap;  ///< -1 = new parm name

  unsigned int itsMaxIter;
  double itsTolerance;
  bool itsPropagateSolutions;
  unsigned int itsSolInt;  ///< Time cell size
  unsigned int itsNChan;   ///< Frequency cell size
  unsigned int itsNFreqCells;

  unsigned int itsTimeSlotsPerParmUpdate;
  unsigned int itsConverged;
  unsigned int itsNonconverged;
  unsigned int itsFailed;
  unsigned int itsStalled;
  std::vector<unsigned int>
      itsNIter;  ///< Total iterations made (for converged, stalled,
                 ///< nonconverged, failed)
  unsigned int itsStepInParmUpdate;  ///< Timestep within parameter update
  double itsChunkStartTime;          ///< First time value of chunk to be stored
  unsigned int itsStepInSolInt;      ///< Timestep within solint

  casacore::Array<casacore::DComplex>
      itsAllSolutions;  ///< Array that holds all solutions for all iterations

  FlagCounter itsFlagCounter;

  NSTimer itsTimer;
  NSTimer itsTimerPredict;
  NSTimer itsTimerSolve;
  NSTimer itsTimerPhaseFit;
  NSTimer itsTimerWrite;
  NSTimer itsTimerFill;
};

}  // namespace DPPP
}  // namespace DP3

#endif
