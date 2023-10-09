# Page Cacheanalyzer

A simple tool to dump the state of the Linux page cache. It needs the
trace-cmd report as its input file.

Run tracing as follows:
```bash
$ trace-cmd record -e mm_filemap_add_to_page_cache -e mm_filemap_delete_from_page_cache -F -c <your-program>
$ trace-cmd report > input_file.txt
```

Run the program as follows:
```bash
$ meson setup build
$ meson compile -C build
$ ./build/cacheanalyzer <device-major-id> input_file.txt
```
