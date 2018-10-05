# Prismriver

.NET wrapper to transcode an input Stream or File into one or more output Streams or Files, optionally adding MetaData.

---

A visual representation of the current Transcoder API:
```
Input                                   Work
System.IO.Stream --\
       or           |-- Prismriver.Transcoder
System.String -----/      |            \          Report Progress (optional)
(^ Filepath)              |             \------- System.IProgress<int, double>
                         [ ] (array of it)        Generics are <Track, Percent>
                          |
                        Prismriver.ITrack (Interface)
Output                   /     \        \              Format (optional if output is a file path)
System.IO.Stream -------/       \        \----- System.String
       or              /         \
System.String --------/           \                  Metadata (Title, Author, etc)
(^ Filepath)                       \----------- System.Object (Not really, just all kind of)
```
