# VMMReadout

This is a (pseudo-)mirror of the NSWELX Readout\_Software repository [here](https://svnweb.cern.ch/cern/wsvn/NSWELX) basically to avoid dealing with SVN so much which has no concept of collaborative code development.

To checkout the trunk of the NSWELx Readout\_Software repo:

```
svn co svn+ssh://svn.cern.ch/reps/NSWELX/Readout_Software/trunk
```

# VMMReadout/Readout_Software

To install the Readout_Software package:

```
cd Readout_Software
source install.sh
make
```

# Tagging in SVN
```
svn cp svn+ssh://${USER}@svn.cern.ch/reps/NSWELX/Readout_Software/trunk/ svn+ssh://${USER}@svn.cern.ch/reps/NSWELX/Readout_Software/tags/Readout_Software-XX-YY-ZZ
```
