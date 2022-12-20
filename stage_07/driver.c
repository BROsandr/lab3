#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  
#include <linux/gpio.h>    
#include <linux/interrupt.h>
#include <linux/jiffies.h>
 
uint8_t kernel_buffer;
uint8_t button;
#define mem_size     1024 
#define GPIO_23_IN  (8) //button is connected to this GPIO

int value = -1;

int notify_param(const char *val, const struct kernel_param *kp)
{
        int res = param_set_int(val, kp);
        if(res==0) {
                printk(KERN_INFO "New value of value = %d\n", value);
                return 0;
        }
        return -1;
}

const struct kernel_param_ops my_param_ops =
{
        .set = &notify_param,
        .get = &param_get_int,
};

module_param_cb(value, &my_param_ops, &value, S_IRUGO|S_IWUSR );

uint8_t led_toggle = 0; 
 
extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;
 
unsigned int GPIO_irqNumber; 
 
static irqreturn_t gpio_irq_handler(int irq,void *dev_id) 
{
  static unsigned long flags = 0;
   unsigned long diff = jiffies - old_jiffie;
   if (diff < 20)
   {
     return IRQ_HANDLED;
   }
  old_jiffie = jiffies;
 
  local_irq_save(flags);
  led_toggle = (0x01 ^ led_toggle);                             
  kernel_buffer = led_toggle;
  local_irq_restore(flags);
  pr_info("Interrupt Occurred : GPIO_23_IN : %d ",gpio_get_value(GPIO_23_IN));
  value = gpio_get_value(GPIO_23_IN);
  return IRQ_HANDLED;
}
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
 
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
 
 
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, 
                char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, 
                const char *buf, size_t len, loff_t * off);
 
static struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .read           = etx_read,
  .write          = etx_write,
  .open           = etx_open,
  .release        = etx_release,
};
 
static int etx_open(struct inode *inode, struct file *file)
{
      pr_info("Device File Opened...!!!\n");
      return 0;
}
 
static int etx_release(struct inode *inode, struct file *file)
{
  pr_info("Device File Closed...!!!\n");
  return 0;
}
 
static ssize_t etx_read(struct file *filp, 
                            char __user *buf, size_t len, loff_t *off)
{
  uint8_t gpio_state = 0;
  gpio_state = gpio_get_value(GPIO_23_IN);
  pr_info("Read function : GPIO_23 = %d \n", gpio_state);
 
  return 0;
 
}
 
static ssize_t etx_write(struct file *filp, 
                const char __user *buf, size_t len, loff_t *off)
{
  pr_info("You can't write to button\n");
  return 0;
}
 
static int __init etx_driver_init(void)
{
  if((alloc_chrdev_region(&dev, 0, 1, "gpio_Dev")) <0){
    pr_err("Cannot allocate major number\n");
    unregister_chrdev_region(dev,1);
    return -1;
  }
  pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
  cdev_init(&etx_cdev,&fops);
 
  if((cdev_add(&etx_cdev,dev,1)) < 0){
    pr_err("Cannot add the device to the system\n");
    cdev_del(&etx_cdev);
    return -1;
  }
 
  if((dev_class = class_create(THIS_MODULE,"gpio_class")) == NULL){
    pr_err("Cannot create the struct class\n");
    class_destroy(dev_class);
    return -1;
  }
 
  if((device_create(dev_class,NULL,dev,NULL,"gpio_device")) == NULL){
    pr_err( "Cannot create the Device \n");
    device_destroy(dev_class,dev);
    return -1;
  }
 
  if(gpio_is_valid(GPIO_23_IN) == false){
    pr_err("GPIO %d is not valid\n", GPIO_23_IN);
    gpio_free(GPIO_23_IN);
    return -1;
  }
 
  if(gpio_request(GPIO_23_IN,"GPIO_23_IN") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_23_IN);
    gpio_free(GPIO_23_IN);
    return -1;
  }
 
  gpio_direction_input(GPIO_23_IN);
 
  GPIO_irqNumber = gpio_to_irq(GPIO_23_IN);  //Get the IRQ number for our GPIO
  pr_info("GPIO_irqNumber = %d\n", GPIO_irqNumber);
 
  if (request_irq(GPIO_irqNumber,             //IRQ number
                  (void *)gpio_irq_handler,   //IRQ handler
                  IRQF_TRIGGER_RISING,        //Handler will be called in raising edge
                  " gpio_device",               //used to identify the device name using this IRQ
                  NULL)) {                    //device id for shared IRQ
    pr_err("gpio_device: cannot register IRQ ");
    gpio_free(GPIO_23_IN);
    return -1;
  }
 
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;
}
 
static void __exit etx_driver_exit(void)
{
  free_irq(GPIO_irqNumber,NULL);
  gpio_free(GPIO_23_IN);
  device_destroy(dev_class,dev);
  class_destroy(dev_class);
  cdev_del(&etx_cdev);
  unregister_chrdev_region(dev, 1);
  pr_info("Device Driver Remove...Done!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivan Ivanov");
MODULE_DESCRIPTION("GPIO button driver");
MODULE_VERSION("1.0");
