// GainCal.cc: DPPP step class to AddNoiseLBA visibilities
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
//
// $Id: GainCal.cc 21598 2012-07-16 08:07:34Z diepen $
//
// @author Tammo Jan Dijkema

#include "AddNoiseLBA.h"

#include <iostream>

#include "../Common/ParameterSet.h"
#include "../Common/Timer.h"

#include <stddef.h>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <random>

using namespace casacore;

namespace DP3 {
  namespace DPPP {

    AddNoiseLBA::AddNoiseLBA (DPInput* input,
                      const ParameterSet& parset,
                      const string& prefix)
    : itsInput(input)
    {
      nsteps = parset.getInt (prefix+"nsteps",10);	   
    }

    AddNoiseLBA::~AddNoiseLBA()
    {}

    void AddNoiseLBA::updateInfo (const DPInfo& infoIn)
    {
      info() = infoIn;
      info().setNeedVisData();
      info().setWriteData();
    }

    void AddNoiseLBA::show (std::ostream& os) const
    {
      os << "AddNoiseLBA " << itsName << endl;
    }

    void AddNoiseLBA::showTimings (std::ostream& os, double duration) const
    {
      os << "  ";
      FlagCounter::showPerc1 (os, itsTimer.getElapsed(), duration);
      os << " AddNoiseLBA " << itsName << endl;
    }

    bool AddNoiseLBA::process (const DPBuffer& buf)
    {
      double nu;    
      double stddev;
      double sefd;
      itsTimer.start();

      // Name of the column to add the noise (at the moment not used, just a placeholder)
      string column = "DATA";
      Array<Complex>::const_contiter indIter = buf.getData().cbegin();

      // Set the exposure
      double exposure = buf.getExposure();

      // Load the Antenna columns
      Vector<Int> antenna1 = info().getAnt1();
      Vector<Int> antenna2 = info().getAnt2();
      //cout << endl;
      //cout << "ANTENNA1 " << antenna1.size() << endl;
      //cout << "ANTENNA2 " << antenna2.size() << endl;

      // Set Number of baselines
      int n_baselines = antenna1.size();
      //cout << "Number of baselines = " << n_baselines << endl;

      // Set the number of correlations
      int n_corr = info().ncorr();
      //cout << "Number of correlations = " << n_corr << endl;

      // Set the LOFAR_ANTENNA_SET
      string antennaSet1 = "LBA_OUTER";
      string antennaSet2 = "LBA_INNER";
      string antennaSet= getInfo().antennaSet();;
      
      // Set the Polynomial coefficients
      double * coeff;
      if (antennaSet.compare(antennaSet1)) coeff = coeffs_outer;
      if (antennaSet.compare(antennaSet2)) coeff = coeffs_inner;

      //int nant = getInfo().antennaNames().size();
      //cout << "N antennas " << nant << endl;

      // Set the number of frequency channels
      int n_freq = getInfo().chanFreqs().size();
      //cout << "Number of frequencies = " << n_freq << endl;
      Vector<Double> chan_freq = getInfo().chanFreqs();
      Vector<Double> chan_width = getInfo().chanWidths();

      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator (seed);

      for (int icorr=1; icorr<n_corr; icorr++)
      {
        for (int ifreq=0; ifreq<n_freq; ifreq++) 
        {
          nu = chan_freq(ifreq);
	  sefd = coeff[0]+
		 coeff[1]*nu+
		 coeff[2]*pow(nu,2.0)+
		 coeff[3]*pow(nu,3.0)+
		 coeff[4]*pow(nu,4.0)+
		 coeff[5]*pow(nu,5.0);
          stddev = eta * sefd;
	  stddev = stddev / sqrt(2.0*exposure*chan_width[ifreq]);
          std::normal_distribution<double> distribution(0.0,stddev);

	  int ibegin = 0;//ifreq*n_baselines + icorr*n_baselines*n_freq;
	  int iend = n_baselines;//ibegin+n_baselines;
	  for (int ibase=ibegin; ibase<iend; ibase++)
	  { 
	      double noise_real = distribution(generator);
	      double noise_img  = distribution(generator);
              std::complex<float> c_noise((float)noise_real, (float)noise_img);
	      std::complex<float> r_noise;
	      r_noise = *indIter + c_noise;
	      indIter++;

          }
	}  

      }

      itsTimer.stop();
      return false;
    }


    void AddNoiseLBA::finish()
    {
      // Let the next steps finish.
      getNextStep()->finish();
    }
  } // end namespace
}
