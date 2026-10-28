/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/mmu.h"

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */  s
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt; // 현재 스레드의 spt를 가져옴

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) { // 페이지를 찾으려 했는데 없을 경우
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		
		struct page *page = malloc(sizeof(struct page)); // 새로운 페이지를 생성

		if (type == VM_ANON) // 타입 비교
		{
			uninit_new(page, upage, init, type, aux, anon_initializer); // 어나니머스 타입이면 anon_initializer를 사용
		}
		else
		{
			uninit_new(page, upage, init, type, aux, file_backed_initializer); // 파일 타입이면 file_backed_initializer 사용
		}

		/* TODO: Insert the page into the spt. */
		return spt_insert_page(spt, page); // spt에 삽입
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function. */
	struct hash_iterator cur;
	hash_first(&cur, spt->spt_hash);
	while (hash_next(&cur))
	{
		struct page *page = hash_entry(hash_cur(&cur), struct page, elem);
		if(page->va == va) return page;
	}

	return page;
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function. */
	struct hash_elem *insert_hash_elem = hash_insert (spt -> spt_hash, &page -> elem);
	if (insert_hash_elem == NULL)
	{
		succ = true;
	}

	return succ;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = malloc(sizeof(struct frame)); // struct frame이나 struct page 같은 경우는 크기가 작기 때문에 크기 설정을 할 수 있는 malloc으로 선언함.
	ASSERT (frame != NULL);
	frame->kva = palloc_get_page(PAL_ZERO||PAL_USER); // palloc을 호출해 사용자 출로 부터 새로운 물리 페이지를 가져옴.
	if(frame->kva == NULL){ // 사용자 풀 메모리가 가득 찼을 경우
		//userpool full so eviction must implement
		PANIC("todo"); // 지금은 vm_evict_frame이 구현되어 있지 않으니 PANIC("TODO")를 호출함.
	}
	
	if(list_empty(&frame_table)){ // frame_table이 비어 있다면
		list_init(&frame_table); // 리스트를 초기화한다.
	}
	list_push_back(&frame_table, &frame->frame_table_elem); // frame_table 리스트에 fream_table_elem을 추가한다.
	
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = spt_find_page(thread_current()->spt, va);
	if(page == NULL){
		return false;
	}
	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();
	/* Set links */
	frame->page = page;
	page->frame = frame;
	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	if(!pml4_set_page(thread_current()->pml4, page, frame, is_writeable(pml4e_walk (thread_current()->pml4, (uint64_t) page, 1)))) // 프로세스의 페이지 테이블에서 가상 페이지의 권한이 쓰기인지 확인을 하고 새 물리 메모리와 가상 페이지 연결이 안 되었다면?
	{
		return false; // false 반환
	}

	return swap_in (page, frame->kva); // 보조기억장치에 있는 내용을 주기억장치로 가져옴.
}

/* Initialize new supplemental page table */
uint64_t 
spt_hash_function (const struct hash_elem *e, void *aux){
	struct page *hash_page = hash_entry(e, struct page, elem);
	return hash_bytes(hash_page->va, sizeof(void *));
}

bool
spt_less_function(const struct hash_elem *a, const struct hash_elem *b, void *aux){
	struct page *pa = hash_entry(a, struct page, elem);
	struct page *pb = hash_entry(b, struct page, elem);
	uint64_t pa_bytes = hash_bytes(pa->va, sizeof(void *));
	uint64_t pb_bytes = hash_bytes(pb->va, sizeof(void *));

	return pa_bytes < pb_bytes;
}

void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	hash_init(spt->spt_hash, spt_hash_function, spt_less_function, NULL);
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}
