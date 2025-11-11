//Creating pseudo 1st char driver

#include<linux/module.h> // necssary for all driver modules
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/export.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>

//Macro to add which function is printing it
#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

#define DEV_MEM_SIZE 256
char device_buffer[DEV_MEM_SIZE]; // pseudo device's memory
dev_t device_number; // Holds the device number
struct cdev pcd_cdev; // Cdev Variable

// file operations of the driver
struct file_operations pcd_fops;
//{
//	.open = pcd_open,
//	.read = pcd_read,
//	.write = pcd_write,
//	.llseek = pcd_seek,
//	.release = pcd_release,
//	.owner = THIS_MODULE
//};



struct class *class_pcd;

struct device *device_pcd;


//Module init function
static int __init pcd_driver_init(void)
{
	// 1.Dynamically allocating a device number
	alloc_chrdev_region(&device_number,0,1,"pcd_devices"); 

	pr_info("Device number <major>:<minor> = %d:%d\n",MAJOR(device_number),MINOR(device_number));
	// 2.Init Cdev structure with this Fops
	cdev_init(&pcd_cdev,&pcd_fops);

	// 3. Register a dveice(cdev structure) with VF
	pcd_cdev.owner = THIS_MODULE;
	cdev_add(&pcd_cdev,device_number,1);

	//4. Create a device class unsder/sys/class/
	class_pcd = class_create("pcd_class");

	//5.Device file creation | Populate sysfs with device information
	device_pcd = device_create(class_pcd,NULL,device_number,NULL,"pcd");

	pr_info("Module Initialized sucessfully\n");
	return 0;
}

//Module exit/clean up function
static void __exit pcd_driver_exit(void)
{
	device_destroy(class_pcd,device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number,1);
	pr_info("Module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lavanya Imadabattina");
MODULE_DESCRIPTION("A simple simple pseudo character driver");
