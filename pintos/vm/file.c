/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include <include/vm/file.h>
#include <include/filesys/filesys.h>
#include <include/threads/vaddr.h>

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Do the mmap */
void *
do_mmap (void *addr, size_t length, int writable, struct file *file, off_t offset) 
{
	/* 
	addr이 NULL이 아닌지, 왜? addr이 NULL이라면 매핑할 주소가 없기 때문에 여부 확인 필요.
	addr을 4kb로 나누었을 때 딱 떨어 맞는지, 왜? 한 페이지가 4kb이기도 하고 페이지 시작 주소 끝이 0 이어야기 때문이다.
	addr이 커널 영역을 넘어오지 않았는지, 왜? 유저 프로그램이 커널 영역을 보는 것을 막기 위함이다.
	length가 0 이하는 아닌지, 왜? length가 0 이하라면 매핑을 하는 의미가 없다.
	확인해서 하나라도 아니라면 return NULL.
	*/

	if ( addr == 0 || (uint64_t)addr % PGSIZE != 0 || is_kernel_vaddr (addr) || length <= 0)
	{
		return NULL;
	}
}

/* Do the munmap */
void
do_munmap (void *addr) {
}
