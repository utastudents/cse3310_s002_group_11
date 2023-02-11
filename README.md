extfat utils is a utility to manipulate extfat images.

# Command Options:
< Keep this up to date >
Allowed options:
1. '-i' option for the input file
2. '-o' option for the output file.
3. '-c' option denotes copy, where the input file is copied to the output file without change
4. '-w' option not avaiable right now,but for future use
5. '-v' option means verify for input file
6. '-h' help text
# Example Invocations:
< Provide examples of use here >


# Internals

## Build Process
```bash
% make clean
% make

To run the tests
% make tests

```

## Examples
There is a directory of examples.  They are compiled by default using the makefile
Before you can run the example programs, you need to have an file image.

```bash
% bash /examples/create_image.bash
% ./examples/fread
% ./examples/mmap
```




















## Misc notes, eventually delete.
```
dd if=/dev/zero of=zzz count=1 bs=1G
yum install -y exfat-utils fuse fuse-exfat
losetup /dev/loop0 diskimage 
mount /dev/loop0 /mnt
umount /mnt
```
## References


https://pawitp.medium.com/notes-on-exfat-and-reliability-d2f194d394c2


https://learn.microsoft.com/en-gb/windows/win32/fileio/exfat-specification


https://uta.service-now.com/selfservice?id=ss_kb_article&sys_id=KB0011414


https://nemequ.github.io/munit/#download


https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html


https://www.freecodecamp.org/news/how-to-use-git-and-github-in-a-team-like-a-pro/


https://en.wikipedia.org/wiki/ExFAT


https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax
