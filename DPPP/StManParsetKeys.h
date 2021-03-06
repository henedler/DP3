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

#ifndef DPPP_STMANPARSETKEYS_H
#define DPPP_STMANPARSETKEYS_H

#include "../Common/ParameterSet.h"

#include "Exceptions.h"

#include <string>

#include <casacore/casa/Containers/Record.h>

#include <boost/algorithm/string/case_conv.hpp>

namespace DP3 {
namespace DPPP {
struct StManParsetKeys {
  casacore::String stManName;
  unsigned int dyscoDataBitRate;  ///< Bits per data float, or 0 if data column
                                  ///< is not compressed
  unsigned int dyscoWeightBitRate;  ///< Bits per weight float, or 0 if weight
                                    ///< column is not compressed
  std::string dyscoDistribution;    ///< Distribution assumed for compression;
                                    ///< e.g. "Uniform" or "TruncatedGaussian"
  double dyscoDistTruncation;  ///< For truncated distributions, the truncation
                               ///< point (e.g. 3 for 3 sigma in TruncGaus).
  std::string
      dyscoNormalization;  ///< Kind of normalization; "AF", "RF" or "Row".

  void Set(const ParameterSet& parset, const std::string& prefix) {
    stManName = boost::to_lower_copy(parset.getString(
        prefix + "storagemanager",
        parset.getString(prefix + "storagemanager.name", string())));
    if (stManName == "dysco") {
      dyscoDataBitRate =
          parset.getInt(prefix + "storagemanager.databitrate", 10);
      dyscoWeightBitRate =
          parset.getInt(prefix + "storagemanager.weightbitrate", 12);
      dyscoDistribution = parset.getString(
          prefix + "storagemanager.distribution", "TruncatedGaussian");
      dyscoDistTruncation =
          parset.getDouble(prefix + "storagemanager.disttruncation", 2.5);
      dyscoNormalization =
          parset.getString(prefix + "storagemanager.normalization", "AF");
    }
  }

  casacore::Record GetDyscoSpec() const {
    casacore::Record dyscoSpec;
    dyscoSpec.define("distribution", dyscoDistribution);
    dyscoSpec.define("normalization", dyscoNormalization);
    dyscoSpec.define("distributionTruncation", dyscoDistTruncation);
    /// DPPP uses bitrate of 0 to disable compression of the data/weight column.
    /// However, Dysco does not allow the data or weight bitrates to be set to
    /// 0, so we set the values to something different. The values are not
    /// actually used.
    unsigned int dataBitRate = dyscoDataBitRate;
    if (dataBitRate == 0) dataBitRate = 16;
    dyscoSpec.define("dataBitCount", dataBitRate);
    unsigned int weightBitRate = dyscoWeightBitRate;
    if (weightBitRate == 0) weightBitRate = 16;
    dyscoSpec.define("weightBitCount", weightBitRate);
    return dyscoSpec;
  }
};
}  // namespace DPPP
}  // namespace DP3
#endif
