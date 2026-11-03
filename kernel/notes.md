# Kernel Notes

## fork behaviour
```
fork()
  mm_copy()
    duplicates VMAs of parent into child
    uvm_copy() -> duplicates present pages
```
if the parent doesn't touch all of his pages, uvm_copy() will not duplicate those in the child.

---
