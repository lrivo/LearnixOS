# fs/ - File Descriptors, Inodes and Pipes

Almost everything a userspace program does with a file - read, write, seek,
dup - is expressed through the filesystem layer. This folder defines the two
abstractions that make that work: the **file descriptor** (`struct file`) and
the **inode** (the on/loaded-bytes "file" it points to), plus the ops-vtable
that lets one syscall serve many different kinds of file.

## The two-layer split

Learnix separates two things that beginners often conflate:

1. **`struct file`** (*what the process holds*) - a small kernel object with an
   independent **offset** and **refcount**, a pointer to some underlying data,
   and a vtable of operations. `proc->fds[i]` holds these; `dup()` duplicates a
   `struct file` **without duplicating the underlying data**.
2. **`struct inode`** (or a TTY, or a pipe) - the *thing* being read/written.
   A `struct file` merely *points* at one (through `f->ptr`) and knows how to
   operate on it (through `f->ops`).

This is the same decoupling that lets real POSIX kernels treat "a file", "a
pipe" and "a serial port" through a single syscall interface.

## `struct file` and `file_ops`

```c
struct file {
  uint64_t offset;          // position of the next read/write
  uint32_t flags;
  uint32_t refcount;        // how many fds currently alias this file
  void    *ptr;             // underlying object (inode, tty, pipe, ...)
  struct file_ops *ops;     // vtable
};
```

The **vtable** (`file_ops`) is where polymorphism lives:

```c
struct file_ops {
  ssize_t (*read) (struct file *, void *buf, size_t count);
  ssize_t (*write)(struct file *, void *buf, size_t count);
  int     (*close)(struct file *);
  off_t   (*lseek)(struct file *, off_t offset, int whence);
};
```

Every "kind" of file brings its own `file_ops` implementation: the TTY
(`sys/tty.c`), the ramfs inode (`fs/inode.c`), and the pipe (`sys/pipe.c`)
each fill in the slots they support and leave the rest `NULL`. This is the
kernel's own mini *virtual method table*, and it's why `sys_read` needs zero
knowledge of what it's reading.

## Lifecycle helper (`fs/file.c`)

- **`file_alloc(ptr, ops)`** - creates a `struct file` with `refcount = 1`
  pointing at the given object.
- **`file_close(f)`** - decrements the refcount; when it hits 0, calls
  `f->ops->close(f)` (if any) and frees the `struct file`. This is why
  `dup()` can share one object among many descriptors safely: no premature
  close.

## Inodes: the ramfs (`fs/inode.c`)

An inode is Learnix's **file**, and for now the inode table is backed by the
**Limine module loader**: the "files" in `/boot/*` are mapped straight from
the bootloader-provided module list.

```c
struct inode {
  vaddr_t  data;     // bytes of the file
  uint64_t size;
  uint32_t inum;
  uint32_t refcount;
};
```

`inode_create(path)` looks up a bootloader module by path and either finds an
existing inode in the small inode table (`itable[]`) or creates a new one.
`inode_read` copies bytes out into the caller's buffer and advances
`f->offset`; `inode_lseek` implements the classic `SEEK_SET/CURR/END` whence
semantics on that offset.

## Pipes (`sys/pipe.c`)

A pipe is a bounded kernel buffer with a **read end** and a **write end**, each
of which is its own `struct file` pointing at the same `struct pipe`:

```c
struct pipe {
  char buffer[PIPE_BUF_SIZE];
  size_t nread, nwrite;   // circular read/write cursor
  size_t nreaders, nwriters;
  struct wait_queue *rq;  // sleep queue for blocked readers
};
```

- **`pipe_read`** - blocks (`sleep_on`) while the pipe is empty but a writer
  exists, and returns **EOF (0)** when no writers remain and nothing is left.
- **`pipe_write`** - returns `-EPIPE` when there are no readers, and wakes up
  blocked readers after copying bytes in.
- **`pipe_close`** - decrements the reader/writer count and frees the shared
  `pipe` once both reach zero.

### `STDIN`/`STDOUT` as files
The boot sequence installs the TTY into `fds[0]` and `fds[1]` as two `struct
file`s pointing at the same `tty0` with *different* `file_ops` (a read-only
vtable and a write-only vtable). That is why the shell `printf` goes to
`stdout` without any special casing in the syscall layer.

> 🐞 The `-1` / `-3` magic return values in `sys_open` and `sys_pipe` are a
> known wart - they should be proper errno constants (tracked in `todo.md`).

Layout notes: `fs/` holds the *abstraction*; the syscall handlers that *drive*
it live in `sys/`. See the [`sys/` README](../sys/README.md).