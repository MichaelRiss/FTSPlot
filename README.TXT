FTSPlot

About:

FTSPlot is a tool for efficiently visualizing large datasets as 
they are common in Electrophysiology/Neuroscience.

System requirements:
- Qt >= 4.7
- cmake >= 2.8
- Linux 64bit or
  Windows 64bit (tested with MSYS/mingw-w64) or
  MacOS X 64bit

Installation:
The installation is based on the cmake configuration utility.

- Create a build directory.
  "mkdir FTSPlot-build"
- Change to the build directory.
  "cd FTSPlot-build" 
- Run the cmake configuration utility with the path of the
  source directory as argument.
  "cmake ../FTSPlot"
- Adapt the configuration to your needs, e.g. make sure you
  do a static link on the binary.
  Available tuning parameters:
  BENCHREPETITIONS: This parameter is only important for benchmarking.
                    It defines how many times the benchmarks are repeated.
  BLOCKFACTOR: This parameter affects the EventEditor and IntervalEditor.
               Block and Node Files will cover index ranges of 2^BLOCKFACTOR.
               The default is 20 and should yield good performance in most
               cases.
  BRANCHFACTOR: This parameter affects the EventEditor and IntervalEditor.
                The directory tree which contains the block and node files
                will branch up with 2^BRANCHFACTOR. 
                The default value is 4 (=> 16 subdirectories) 
		which yields good performance in most cases.
  COUNT_TILES: When benchmarking also write out statistics about the amount
               of vertices, lines and quad elements generated per display list.
  DISABLE_LINUX_FILESYSTEMCACHE: When benchmarking on Linux empty file system cache 
                                 between two benchmarks. Helps to get results closer to 
                                 "cold start" behavior but cannot empty caches on the
                                 hard disks.
- "make"
The binaries can then be found in the "build"-directory. Copy them
to the destination folder of choice.


Short tutorial on visualizing timeseries datasets:

FTSPrep
Before visualizing a timeseries dataset in "FTSPlot" the dataset needs to
be prepared. This is done with the program "FTSPrep". As the preparation 
of big timeseries datasets can take quite some time, the program is 
separated from the main program and can process several datafiles
in batch mode.
Please note that currently only single channel, native endian, 
double valued datasets are supported.
Select the desired reduction factor, 64 is the recommended default.
Add one or several datasets to the list and then press "Start".
In case the process needs to be stopped a resume file can be saved
so that the preparation process resumes at that point the next time.
The preparation process generates several additional files alongside the
original dataset. The reduced datasets carry number suffixes, the 
configuration file for the dataset carries the "*.cfg" suffix. It
is used to open the dataset in FTSPlot.


FTSPlot
After starting the program the main window with the plotting area appears. 
Open the module manager by clicking on the "Menu" button.
In the module manager click on the "Add TimeSeries" button and select
the previously generated "*.cfg" file.
You should now see the timeseries dataset in the plot area of the main 
window. The following navigation controls are available:
- Press and hold left mouse butten inside the plotting area and move mouse
  left/right to pan the view.
- Use the mouse wheel to zoom the view in x-direction.
- Hold CTRL and left mouse button to pan the view in x- and y-direction.
- Hold CTRL and use the mouse wheel to zoom the view in y-direction.



Detailed Explanation of the Controls:

Main Window:
The main window contains the plotting area. With the "Menu" button the 
module manager window can be toggled on or off. By closing the main
window the program terminates.
Plotting area controls:
- Press and hold left mouse butten inside the plotting area and move mouse
  left/right to pan the view.
- Use the mouse wheel to zoom the view in x-direction.
- Hold CTRL + left mouse button and move the mouse to pan the view in 
  x- and y-direction.
- Hold CTRL and use the mouse wheel to zoom the view in y-direction.
If an EventList or IntervalList module is active the SHIFT key directs the 
following mouse clicks to the active module:
- In "Add Event" or "Add Interval" mode hold SHIFT and use the left mouse
  button to add events or intervals. For intervals you need to mark two points
  for the beginning and the end of an interval.
- In "Select Event" or "Select Interval" mode hold SHIFT and use the left
  mouse button to select an event/interval.
- In "Delete Event" or "Delete Interval" mode hold SHIFT and use the left 
  mouse button to delete an event/interval.


Module Manager:
In the module manager window visualization modules can be added, deleted
and manipulated. 
On the left side there are buttons for adding timeseries, eventlist and 
intervallist modules.
The modules appear in the list to the right. To delete a module, select it
in the list and press "Delete Module" on the left side.
The plotting order of the modulues in the list can be changed by selecting
a module and changing its position in the list with the "Up"/"Down" buttons.
The checkbox in the "Graph" column toggles the visibility of the module in
the plotting area. 
With a double click on the "Color" field of a module a new color can be 
assigned to the visualization module.
The "Name" field contains an identification of the module. Changes
to the name field are only temporary and do not get saved.
EventList and IntervalList modules have two further options available:
The "GUI" checkbox toggles the extra control panel of the corresponding 
visualization module. The "Edit Active" radio button selects the module
that receives editing input from the plot area of the main window.


Event Editor:
Use "Open EventList" to open either an existing event list directory or 
to create a new one. For creating a new one, use the file browser to 
create a new directory and then select it. A new Eventlist data structure 
will be created in the directory.
"Close EventList" is closing the EventList data structure.
Import/Export:
With "Import Flat File" and "Export Flat File" the EventList data structure
can be loaded and saved into a single bulk file. This is useful for
processing event lists in other programs and scripts.
The events in the bulk file are saved as a series of ascendingly ordered 
binary unsigned 64 bit native endian integer values. Each integer value represents
the sample position of an event.
With "Add Event at" a new event can be added at the given sample position.
Plot area editing:
The three radio buttons "Add Event", "Select Event" and "Delete Event" 
in the lower left area control the way mouse input in the plot area in the
main window is interpreted. With "Add Event" a SHIFT + mouse click in the
plot area adds a new event to the event list. With "Delete Event" events in 
the plot area can be deleted by SHIFT+clicking on them. "Select Event" lets
you choose a new current event by SHIFT+clicking it.
Current Event Controls:
As long as there is at least one event in the event list the event editor 
keeps track of the current event. It can be selected with the mouse in the
plot area by using "Select Event".  In the Finetuning area it can be 
moved up/down with the "+"/"-" buttons or deleted ("Delete").
With the buttons "Previous Event" and "Next Event" the current event is
set to the previous or next event in the event list.
If the "Track in View" checkbox is marked the plot area is centered onto 
the new current event location, this is useful for quickly inspecting 
an event list.


Interval Editor:
The Interval Editor control panel is analogous to the Event Editor panel.
Use "Open IntervalList" to open/create a new IntervalList.
"Close IntervalList" to close and disconnect from the current Interval List
data structure.
"Import Flat File" and "Export Flat File" reads from or creates a flat bulk file.
The flat file consists of pairs of binary unsigned 64bit native endian integer
values. In "C-code":
typedef struct
{
    unsigned long begin;
    unsigned long end;
} interval;
The first value (begin) marks the sample number of the beginning of the interval
the second value the end of the interval. "begin" needs to be smaller or equal 
to "end" (begin <= end).
The intervals within the flat file have to be ordered ascendingly. The order is 
defined in "C-code" as follows:
bool lessThan( interval v1, interval v2 )
{
    if( v1.begin != v2.begin )
    {
      return v1.begin < v2.begin;
    }
    else
    {
      return v1.end < v2.end;
    }
}
As long as the "begin" values differ the order is equal to the order on the 
"begin" values. Only if the "begin" values are identical the order is based on the
"end" values.
With "Add Interval at" and the two fields on top a new interval can be added at 
the given sample position.
Plot area editing:
The three radio buttons "Add Interval", "Select Interval" and "Delete Interval" 
in the lower left area control the way mouse input in the plot area in the
main window is interpreted. With "Add Interval" two SHIFT + mouse clicks in the
plot area add a new interval to the interval list. With "Delete Interval" intervals in 
the plot area can be deleted by SHIFT+clicking on them. "Select Interval" lets
you choose a new current interval by SHIFT+clicking it.
Current Interval Controls:
As long as there is at least one interval in the interval list the interval editor 
keeps track of the current interval. It can be selected with the mouse in the
plot area by using "Select Interval". In the Finetuning area the beginning and the end 
can be moved up/down with the "+"/"-" buttons or deleted ("Delete Current Interval").
With the buttons "Previous Interval" and "Next Interval" the current interval is
set to the previous or next interval in the interval list.
If the "Track in View" checkbox is marked the plot area is centered onto 
the new current interval location, this is useful for quickly inspecting 
an interval list. If "Fit Scale" is selected the zoom factor gets also adapted to
the new current interval.


Tests:
The "runTests" binary comprises a rudimentary test suite for the FTSPlot library.
While being far from exhaustive these basic tests should pass on any platform.


Benchmarks:
"FTSPlotBench" is a tool to run benchmarks. Three different types of benchmarks 
can be selected, Timeseries, EventEditor or IntervalEditor benchmarks.
Select the desired dataset and a directory for the benchmark results and 
click on the corresponding "Start benchmark" button.
In the results directory several files get generated. Depending on the benchmark
type ("TimeSeries", "EventEditor" or "IntervalEditor") they start with the corresponding
file name. 
The number suffix corresponds to the width of the visualized data. So a *.1 file
corresponds to the visualization of samples 0 and 1 while *.2048 files
correspond to the visualization of samples 0 to 2048. The number of samples increases in
powers of 2.
Two types of benchmarks are executed.
In the first one the OpenGL display list is created once and then painted multiple 
times. The paint times for this benchmark are saved in the "PurePaint" files.
This benchmark type has the flaw that the display list creation time is not measured and 
that smart OpenGL implementations might figure out that they are painting the same
display list over and over again and activate extra optimizations that can have an
influence on the result.
In the second benchmark type the OpenGL display list is created, painted and then 
deleted. The display list creation time is measured and saved in the "displayList" files, 
the paint times are saved in the "Paint" files.
The benchmarks are repeated several times depending on the BENCHREPETITIONS parameter
in the cmake configuration. For each repetition a timing value is written into the files.
E.g. with the default BENCHREPETITIONS parameter of 1000 each file should contain 1000
timed benchmark measurements. The timings are measured in nano seconds.
