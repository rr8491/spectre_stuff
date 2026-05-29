#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x5fa7ad45, "module_layout" },
	{ 0xa7e590be, "__tracepoint_mmap_lock_released" },
	{ 0x92f949f7, "misc_deregister" },
	{ 0xaf9a8d7a, "misc_register" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x92997ed8, "_printk" },
	{ 0x77ab2e5c, "__mmap_lock_do_trace_acquire_returned" },
	{ 0x27ad572c, "__mmap_lock_do_trace_start_locking" },
	{ 0x7d957984, "find_vma" },
	{ 0xfe4623d2, "__tracepoint_mmap_lock_acquire_returned" },
	{ 0x668b19a1, "down_read" },
	{ 0xbd971f79, "__tracepoint_mmap_lock_start_locking" },
	{ 0xdebdb799, "current_task" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x53b954a2, "up_read" },
	{ 0x51d0ef6a, "__mmap_lock_do_trace_released" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "E5C8120A65CC243FB9945FB");
