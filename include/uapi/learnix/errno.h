#pragma once

/* Linux syscall errors (they are returned negated) */
#define EBADF 9     // bad file descriptor
#define EFAULT 14   // invalid address
#define EEXIST 17   // already exist
#define EINVAL 22
#define EPIPE 32    // broken pipe
#define EPERM 1     // operation not permitted
#define ENOTSUP 95  // operation not supported
