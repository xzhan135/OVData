These are the diffuse and specular reflectance measurements of Panel 001-090.
There are 10% of the panel get measured. Each of the panel have been measured three times on each side.

The analyzer must be compiled with root6 and c++ > 4.9.2.

To analyze the data, one needs to 
$ ls /names/of/the/data/files >> /the/list/of/data/file.list
$ root -l -x 'OVAnalysis("</the/list/of/data/file.list>")'

Then it will show the .root file that stores all the TH1D generated. Try play with them!
