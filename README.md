<h1> structuredLogger</h1>
<h3> c++ library  for program logging with a consitent message structure.  Built upon boost::logger </h3>
<p> small library that allows you to have structured log messages. 
</p>
<ul>Installation:  
<ul> source files  
<li> structuredlogger.cpp/h stackTrace.cpp/h</li>
</ul>
<ul> testdirectory
<li> test main.cpp,msgFormatVerify.cpp/h, stackdumptest.cpp/h </li>
</ul>
<ul> requirements: 
<li> boost libraries </li>
<li> phthreads library </li>
</ul>
<li> make all -f structuredLogger.cbp.mak or use codeblocks project file </li>
<li> documentation can be generated from doxygen </>
</ul>
</ul>
<h2>Features</h2>
<ul>
<li> choice of logging to 1-3 files, log, error, and debug </li>
<li> asynchronous logging </li> 
<li> microsecond time stamp </li> 
<li> filtered logging by severity </li> 
</ul>
<h3> examples </h3> 
<code>record-number date severity [token] message  </code><br>
<code> 4 2017-03-06 14:48:09.108104 \<notification \> [void s_StructruedLogging::general_logging::test_method()] msg info GEN </code> <br>
<code> 24 2017-03-06 14:48:09.119579 \<debug \> [void s_StructruedLogging::debugLogging::test_method()] [0x00007ffff7fea740] msg debug DEBUG </code> 

