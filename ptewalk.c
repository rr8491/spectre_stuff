#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <asm/tlbflush.h>

#define IOCTL_CHECK_PTE _IOW('p', 1, unsigned long)
#define IOCTL_CLEAR_YOUNG  _IOW('p', 2, unsigned long)

// pte is 8 bytes
static int inspect_pte(unsigned long addr, bool clear_young )
{
    struct mm_struct *mm = current->mm;

    struct vm_area_struct *vma;


    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    pte_t pteval;

    if (!mm) {
        pr_err("ptewalk: current process has no mm_struct\n");
        return -EINVAL;
    }

    mmap_read_lock(mm);


    vma = find_vma(mm, addr);

        if (!vma || addr < vma->vm_start) {
        pr_info("ptewalk: no VMA for addr=%lx\n", addr);
        goto not_present;
    }

    pgd = pgd_offset(mm, addr); // page global dir entry which corresponds to the mmapped addr
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        pr_info("ptewalk: bad/missing PGD for addr=%lx\n", addr);
        goto not_present;
    }

    // level 2: P4D.
    p4d = p4d_offset(pgd, addr); // page level 4 directory offset using the pgd entry
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        pr_info("ptewalk: bad/missing P4D for addr=%lx\n", addr);
        goto not_present;
    }

    // level 3: PUD.
    pud = pud_offset(p4d, addr);  // level 3 page upper directory offset using the p4d entry
    if (pud_none(*pud) || pud_bad(*pud)) {
        pr_info("ptewalk: bad/missing PUD for addr=%lx\n", addr);
        goto not_present;
    }


    // level 4: PMD.
    pmd = pmd_offset(pud, addr); // level 2 page middle directoty offset using the pmd entry
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        pr_info("ptewalk: bad/missing PMD for addr=%lx\n", addr);
        goto not_present;
    }

    if (pmd_trans_huge(*pmd)) {
    pr_info("ptewalk: addr=%lx is mapped by transparent huge PMD page\n", addr);
    mmap_read_unlock(mm);
    return 0;
  }



   // level 5 
    pte = pte_offset_kernel(pmd, addr);
    if (!pte) {
        pr_info("ptewalk: could not map PTE for addr=%lx\n", addr);
        goto not_present;
    }

  
    pteval = *pte;


    if (!pte_present(pteval)) {
        pr_info("ptewalk: PTE exists but page is not present for addr=%lx\n", addr);
        goto not_present;
    }

        if (clear_young) {

        pte_t oldpte = pteval;

        pte_t newpte = pte_mkold(oldpte);

        set_pte_at(mm, addr, pte, newpte);

        //__flush_tlb_one_user(addr);
   
        pr_info("ptewalk: cleared accessed bit addr=%lx old_pte=%lx new_pte=%lx old_young=%d new_young=%d\n",
                addr,
                pte_val(oldpte),
                pte_val(newpte),
                !!pte_young(oldpte),
                !!pte_young(newpte));

   
        pteval = newpte;
    }


    // https://www.kernel.org/doc/gorman/html/understand/understand006.html
    // pte_val(pteval): raw PTE bits.
    // pte_present(): whether it maps present memory.
    // pte_young(): Linux's name for the accessed bit.
    // pte_dirty(): dirty/write bit.
    // pte_write(): writable bit.


    pr_info("ptewalk: addr=%lx raw_pte=%lx present=%d accessed/young=%d dirty=%d write=%d\n",
        addr,
        pte_val(pteval),
        pte_present(pteval),
        pte_young(pteval),
        pte_dirty(pteval),
        pte_write(pteval));

    // Release the mmap read lock.
    mmap_read_unlock(mm);

    return 0;

not_present:
    
    mmap_read_unlock(mm);
    return -EINVAL;
}

// ioctl entry point for /dev/ptewalk.
static long ptewalk_ioctl(struct file *file,
                          unsigned int cmd,
                          unsigned long arg)
{
    switch (cmd) {

    //only inspect PTE.
     
    case IOCTL_CHECK_PTE:
        return inspect_pte(arg, false);

    
     //inspect and clear accessed bit.
     
    case IOCTL_CLEAR_YOUNG:
        return inspect_pte(arg, true);

    default:
        return -EINVAL;
    }
}


static const struct file_operations ptewalk_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = ptewalk_ioctl,
};

// register as a misc device.
static struct miscdevice ptewalk_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "ptewalk",
    .fops = &ptewalk_fops,
};

// called when module is loaded with insmod.
static int __init ptewalk_init(void)
{
    int ret;

    ret = misc_register(&ptewalk_dev);
    if (ret) {
        pr_err("ptewalk: failed to register misc device\n");
        return ret;
    }

    pr_info("ptewalk: loaded, device created at /dev/ptewalk\n");
    return 0;
}

// called when module is removed with rmmod.
static void __exit ptewalk_exit(void)
{
    misc_deregister(&ptewalk_dev);
    pr_info("ptewalk: unloaded\n");
}

module_init(ptewalk_init);
module_exit(ptewalk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("Walk current process page tables and print PTE info");
