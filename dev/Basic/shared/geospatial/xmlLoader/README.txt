#Some commands to help you get started with XSD

#xsdcxx  --options-file  


#If you update the geo9.xsd file, you should run the following:
xsd  cxx-parser --root-element-last --hxx-suffix .hpp --cxx-suffix .cpp --type-map geoConverter.map --output-dir skeleton   geo9.xsd

#The "cxx-parser" tells XSD to generate a C++ parser (as opposed to a C++ tree). We use parsers so that we can load data as it 
#  arrives, rather than loading the entire XML tree into memory at once.

#The option "--root-element-last" tells the parser to only treat the *last* global element as a document root (instead of 
#  all global elements).

#The option "--type-map" allows us to specify XML to C++ mappings. (It's complicated; see the XSD tutorial)

#The "--hxx-suffix" and "--cpp-suffix" options are used to give us the suffixes hpp and cpp for our generated files.

#The option "--output-dir skeleton" will put the generated (skeleton) files in the skeleton directory, in order to
#  clearly separate them from the implementation files.

#Running this command will generate the files "geo9-pskel.hpp/cpp", which contain the "skeleton" implementation of
#  the parser. You should then (manually) update the geo9-pimpl.hpp/cpp files to match. Remember that XSD accomplishes
#  all of its loading through inheritance. 
#The geo9-driver.cpp/hpp file may also have to be (manually) updated, but this code will probably be migrated out into 
#  multiple files in the future. 


#TODO: 
#TODO: --namespace-map http://www.smart.mit.edu/geo=sim_mob::xml
#TODO: geo9 is in the temp directory... what to do about geo9?


#The VERY FIRST TIME you start development for a new type of object, you can run the following:
xsd cxx-parser --root-element-last --generate-print-impl  --generate-test-driver  --hxx-suffix .hpp --cxx-suffix .cpp --type-map geoConverter.map geo9.xsd

#The option "--generate-print-impl" will create geo9-pimpl.hpp/cpp, which extend the skeleton classes with functions which print each 
#  item as it is read.

#The option "--generate-test-driver" will create geo9-driver.cpp, which contains a main file which will run through the pimpl classes.

#Running this will generate *all* files, including the implementation/driver files. This is not as useful as it sounds, because it 
#  will overwrite your existing implementation/driver files. (Well, it will actually not do that, unless you --force it to.) 
#This can be useful, though, if you direct this command to a temporary "--output-dir" and then only copy the pieces of code that
#  you need over to the existing implementation/driver files.  


