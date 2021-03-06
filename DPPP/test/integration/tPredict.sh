#!/bin/bash

# Locate the executables and srcdir (script created by cmake's configure_file).
INIT=testInit.sh
if [ ! -f $INIT ]; then
  echo $INIT not found. Please run this script from build/DPPP/test.
  exit 1;
fi
source $INIT

tar zxf ${srcdir}/tPredict.tab.tgz

# Create expected taql output.
echo "    select result of 0 rows" > taql.ref

echo; echo "Test with beam, subtract"; echo
cmd="$dpppexe msin=tNDPPP-generic.MS msout=. msout.datacolumn=MODEL_DATA steps=[predict] predict.sourcedb=tNDPPP-generic.MS/sky predict.usebeammodel=true predict.operation=subtract"
echo $cmd
$cmd
# Compare the MODEL_DATA column of the output MS with the original data minus the BBS reference output.
taqlcmd='select from tNDPPP-generic.MS t1, tPredict.tab t2 where not all(near(t1.MODEL_DATA,t1.DATA-t2.PREDICT_beam,5e-2) || (isnan(t1.DATA) && isnan(t2.PREDICT_beam)))'
echo $taqlcmd
$taqlexe $taqlcmd > taql.out
diff taql.out taql.ref  ||  exit 1

echo; echo "Test without beam, add"; echo
cmd="$dpppexe msin=tNDPPP-generic.MS msout=. msout.datacolumn=MODEL_DATA steps=[predict] predict.sourcedb=tNDPPP-generic.MS/sky predict.usebeammodel=false predict.operation=add"
echo $cmd
$cmd
# Compare the MODEL_DATA column of the output MS with the original data plus the BBS reference output.
taqlcmd='select from tNDPPP-generic.MS t1, tPredict.tab t2 where not all(near(t1.MODEL_DATA,t1.DATA+t2.PREDICT_nobeam,5e-2) || (isnan(t1.DATA) && isnan(t2.PREDICT_beam)))'
echo $taqlcmd
$taqlexe $taqlcmd > taql.out
diff taql.out taql.ref  ||  exit 1

#This sub-test is disabled since it uses parmdbm, which is deprecated.
#
#echo; echo "Test without beam, with applycal, subtract (like peeling)"; echo
#parmdbm <<EOL
#open table="tPredict.parmdb"
#adddef Gain:0:0:Real values=3.
#adddef Gain:1:1:Real values=3.
#EOL
#cmd="$dpppexe msin=tNDPPP-generic.MS msout=. msout.datacolumn=MODEL_DATA steps=[predict] predict.sourcedb=tNDPPP-generic.MS/sky predict.applycal.parmdb=tPredict.parmdb predict.operation=subtract"
#echo $cmd
#$cmd
## Compare the MODEL_DATA column of the output MS with the original data minus the BBS reference output.
#taqlcmd='select from tNDPPP-generic.MS t1, tPredict.tab t2 where not all(near(t1.MODEL_DATA,t1.DATA-9.0*t2.PREDICT_nobeam,5e-2) || (isnan(t1.DATA) && isnan(t2.PREDICT_nobeam)))'
#echo $taqlcmd
#$taqlexe $taqlcmd > taql.out
#diff taql.out taql.ref  ||  exit 1

echo; echo "Test without beam"; echo
cmd="$dpppexe msin=tNDPPP-generic.MS msout=. steps=[predict] predict.sourcedb=tNDPPP-generic.MS/sky"
echo $cmd
$cmd
# Compare the DATA column of the output MS with the BBS reference output.
taqlcmd='select from tNDPPP-generic.MS t1, tPredict.tab t2 where not all(near(t1.DATA,t2.PREDICT_nobeam,5e-2) || (isnan(t1.DATA) && isnan(t2.PREDICT_nobeam)))'
echo $taqlcmd
$taqlexe $taqlcmd > taql.out
diff taql.out taql.ref  ||  exit 1

echo; echo "Test with beam"; echo
cmd="$dpppexe msin=tNDPPP-generic.MS msout=. steps=[predict] predict.sourcedb=tNDPPP-generic.MS/sky predict.usebeammodel=true"
echo $cmd
$cmd
# Compare the DATA column of the output MS with the BBS reference output.
taqlcmd='select from tNDPPP-generic.MS t1, tPredict.tab t2 where not all(near(t1.DATA,t2.PREDICT_beam,5e-2) || (isnan(t1.DATA) && isnan(t2.PREDICT_beam)))'
echo $taqlcmd
$taqlexe $taqlcmd > taql.out
diff taql.out taql.ref  ||  exit 1
