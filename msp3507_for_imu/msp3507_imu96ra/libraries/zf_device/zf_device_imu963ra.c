/*********************************************************************************************************************
* MSPM0G3507 Opensource Library ???MSPM0G3507 ??????????????????SDK ??????????????
* Copyright (c) 2022 SEEKFREE ??????
* 
* ?????? MSPM0G3507 ???????????
* 
* MSPM0G3507 ????? ????????* ???????????????????????GPL??NU General Public License??? GNU?????????????????* ??GPL ???3?????GPL3.0??????????????????????????????????????
* 
* ??????????????????????????????????????????
* ???????????????????????????????
* ???????????GPL
* 
* ??????????????????????????GPL ?????* ????????????<https://www.gnu.org/licenses/>
* 
* ????????* ????????? GPL3.0 ??????????? ?????????????????* ???????????? libraries/doc ????????GPL3_permission_statement.txt ?????* ????????? libraries ?????? ???????????LICENSE ???
* ?????????????????? ??????????????????????????????????????* 
* ??????          zf_device_imu963ra
* ??????          ???????????????
* ??????          ??? libraries/doc ?????? version ??? ??????
* ???????         MDK 5.37
* ??????          MSPM0G3507
* ??????          https://seekfree.taobao.com/
* 
* ??????
* ???              ????               ???
* 2025-06-1        SeekFree            first version
********************************************************************************************************************/
/*********************************************************************************************************************
* ????????*                   ------------------------------------
*                   ??????            ????????*                   // ??? SPI ???
*                   SCL/SPC             ??? zf_device_imu963ra.h ??IMU963RA_SPC_PIN ?????*                   SDA/DSI             ??? zf_device_imu963ra.h ??IMU963RA_SDI_PIN ?????*                   SA0/SDO             ??? zf_device_imu963ra.h ??IMU963RA_SDO_PIN ?????*                   CS                  ??? zf_device_imu963ra.h ??IMU963RA_CS_PIN  ?????*                   VCC                 3.3V???
*                   GND                 ?????*                   ?????????
*
*                   // ??? IIC ???
*                   SCL/SPC             ??? zf_device_imu963ra.h ??IMU963RA_SCL_PIN ?????*                   SDA/DSI             ??? zf_device_imu963ra.h ??IMU963RA_SDA_PIN ?????*                   VCC                 3.3V???
*                   GND                 ?????*                   ?????????
*                   ------------------------------------
********************************************************************************************************************/

#include "zf_common_clock.h"
#include "zf_common_debug.h"
#include "zf_driver_delay.h"
#include "zf_driver_spi.h"
#include "zf_driver_soft_iic.h"

#include "zf_device_imu963ra.h"

int16 imu963ra_gyro_x = 0, imu963ra_gyro_y = 0, imu963ra_gyro_z = 0;
int16 imu963ra_acc_x = 0,  imu963ra_acc_y = 0,  imu963ra_acc_z = 0;
int16 imu963ra_mag_x = 0,  imu963ra_mag_y = 0,  imu963ra_mag_z = 0;
float imu963ra_transition_factor[3] = {4098, 14.3, 3000};

#if IMU963RA_USE_SOFT_IIC
static soft_iic_info_struct imu963ra_iic_struct;

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ??????
// ??????     reg             ????????
// ??????     data            ???
// ??????     void
// ??????     imu963ra_write_acc_gyro_register(IMU963RA_SLV0_CONFIG, 0x00);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
#define imu963ra_write_acc_gyro_register(reg,data)       (soft_iic_write_8bit_register(&imu963ra_iic_struct,reg,data))

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ??????
// ??????     reg             ????????
// ??????     uint8           ???
// ??????     imu963ra_read_acc_gyro_register(IMU963RA_STATUS_MASTER);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
#define imu963ra_read_acc_gyro_register(reg)             (soft_iic_sccb_read_register(&imu963ra_iic_struct,reg))

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ???????????
// ??????     reg             ????????
// ??????     data            ????????// ??????     len             ??????
// ??????     void
// ??????     imu963ra_read_acc_gyro_registers(IMU963RA_OUTX_L_A, dat, 6);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
#define imu963ra_read_acc_gyro_registers(reg,data,len)   (soft_iic_read_8bit_registers(&imu963ra_iic_struct,reg,data,len))
#else
//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ??????
// ??????     reg             ????????
// ??????     data            ???
// ??????     void
// ??????     imu963ra_write_acc_gyro_register(IMU963RA_SLV0_CONFIG, 0x00);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
static void imu963ra_write_acc_gyro_register (uint8 reg, uint8 data)
{
    IMU963RA_CS(0);
    spi_write_8bit_register(IMU963RA_SPI, reg | IMU963RA_SPI_W, data);

    IMU963RA_CS(1);
}

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ??????
// ??????     reg             ????????
// ??????     uint8           ???
// ??????     imu963ra_read_acc_gyro_register(IMU963RA_STATUS_MASTER);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
static uint8 imu963ra_read_acc_gyro_register (uint8 reg)
{
    uint8 data = 0;
    IMU963RA_CS(0);
    data = spi_read_8bit_register(IMU963RA_SPI, reg | IMU963RA_SPI_R);

    IMU963RA_CS(1);
    return data;
}

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ???????????
// ??????     reg             ????????
// ??????     data            ????????// ??????     len             ??????
// ??????     void
// ??????     imu963ra_read_acc_gyro_registers(IMU963RA_OUTX_L_A, dat, 6);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
static void imu963ra_read_acc_gyro_registers (uint8 reg, uint8 *data, uint32 len)
{
    IMU963RA_CS(0);
    spi_read_8bit_registers(IMU963RA_SPI, reg | IMU963RA_SPI_R, data, len);

    IMU963RA_CS(1);
}
#endif

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ??? IIC ??????????????// ??????     addr            ??????
// ??????     reg             ????????// ??????     data            ???
// ??????     uint8           1-??? 0-???
// ??????     imu963ra_write_mag_register(IMU963RA_MAG_ADDR, IMU963RA_MAG_CONTROL2, 0x80);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
static uint8 imu963ra_write_mag_register (uint8 addr, uint8 reg, uint8 data)
{
    uint8 return_state = 0;
    uint16 timeout_count = 0;

    addr = addr << 1;
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_CONFIG, 0x00);               // ???0??????
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_ADD, addr | 0);              // ????????????????????????????I2C?????0x2C
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_SUBADD, reg);                // ????????????????
    imu963ra_write_acc_gyro_register(IMU963RA_DATAWRITE_SLV0, data);            // ???????????
    imu963ra_write_acc_gyro_register(IMU963RA_MASTER_CONFIG, 0x4C);             // ????????????????? ???????I2C??????
    
    // ?????????
    while(0 == (0x80 & imu963ra_read_acc_gyro_register(IMU963RA_STATUS_MASTER)))
    {
        if(IMU963RA_TIMEOUT_COUNT < timeout_count ++)
        {
            return_state = 1;
            break;
        }
        system_delay_ms(2);
    }
    return return_state;
}

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ??? IIC ??????????????// ??????     addr            ??????
// ??????     reg             ????????// ??????     uint8           ????????// ??????     imu963ra_read_mag_register(IMU963RA_MAG_ADDR, IMU963RA_MAG_CHIP_ID);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
static uint8 imu963ra_read_mag_register (uint8 addr, uint8 reg)
{
    uint16 timeout_count = 0;

    addr = addr << 1;
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_ADD, addr | 1);              // ????????????????????????????I2C?????0x2C
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_SUBADD, reg);                // ????????????????
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_CONFIG, 0x01);    
    imu963ra_write_acc_gyro_register(IMU963RA_MASTER_CONFIG, 0x4C);             // ????????????????? ???????I2C??????
    
    // ?????????
    while(0 == (0x01 & imu963ra_read_acc_gyro_register(IMU963RA_STATUS_MASTER)))
    {
        if(IMU963RA_TIMEOUT_COUNT < timeout_count ++)
        {
            break;
        }
        system_delay_ms(2);
    }
    
    return (imu963ra_read_acc_gyro_register(IMU963RA_SENSOR_HUB_1));            // ????????????
}

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ??? IIC ?????????????????// ??????     addr            ??????
// ??????     reg             ????????// ??????     void
// ??????     imu963ra_connect_mag(IMU963RA_MAG_ADDR, IMU963RA_MAG_OUTX_L);
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
static void imu963ra_connect_mag (uint8 addr, uint8 reg)
{
    addr = addr << 1;
    
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_ADD, addr | 1);              // ????????????????????????????I2C?????0x2C
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_SUBADD, reg);                // ????????????????
    imu963ra_write_acc_gyro_register(IMU963RA_SLV0_CONFIG, 0x06);    
    imu963ra_write_acc_gyro_register(IMU963RA_MASTER_CONFIG, 0x6C);             // ????????????????? ???????I2C??????
}   


//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ?????? ??????
// ??????     void
// ??????     uint8           1-?????? 0-??????
// ??????     imu963ra_acc_gyro_self_check();
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
static uint8 imu963ra_acc_gyro_self_check (void)
{
    uint8 return_state = 0;
    uint8 dat = 0;
    uint16 timeout_count = 0;

    while(0x6B != dat)                                                          // ??? ID ??????
    {
        if(IMU963RA_TIMEOUT_COUNT < timeout_count ++)
        {
            return_state = 1;
            break;
        }
        dat = imu963ra_read_acc_gyro_register(IMU963RA_WHO_AM_I);
        system_delay_ms(10);
    }
    return return_state;
}

//-------------------------------------------------------------------------------------------------------------------
// ???????    IMU963RA ???????? ??????
// ??????     void
// ??????     uint8           1-?????? 0-??????
// ??????     imu963ra_mag_self_check();
// ??????     ??????
//-------------------------------------------------------------------------------------------------------------------
static uint8 imu963ra_mag_self_check (void)
{
    uint8 return_state = 0;
    uint8 dat = 0;
    uint16 timeout_count = 0;

    while(0xff != dat)                                                          // ??? ID ??????
    {
        if(IMU963RA_TIMEOUT_COUNT < timeout_count ++)
        {
            return_state = 1;
            break;
        }
        dat = imu963ra_read_mag_register(IMU963RA_MAG_ADDR, IMU963RA_MAG_CHIP_ID);
        system_delay_ms(10);
    }
    return return_state;
}

//-------------------------------------------------------------------------------------------------------------------
// ???????    ??? IMU963RA ??????????// ??????     void
// ??????     void
// ??????     imu963ra_get_acc();
// ??????     ???????????????????????????
//-------------------------------------------------------------------------------------------------------------------
void imu963ra_get_acc (void)
{
    uint8 dat[6];

    imu963ra_read_acc_gyro_registers(IMU963RA_OUTX_L_A, dat, 6);
    imu963ra_acc_x = (int16)(((uint16)dat[1] << 8 | dat[0]));
    imu963ra_acc_y = (int16)(((uint16)dat[3] << 8 | dat[2]));
    imu963ra_acc_z = (int16)(((uint16)dat[5] << 8 | dat[4]));
}


//-------------------------------------------------------------------------------------------------------------------
// ???????    ???IMU963RA????????
// ??????     void
// ??????     void
// ??????     imu963ra_get_gyro();
// ??????     ???????????????????????????
//-------------------------------------------------------------------------------------------------------------------
void imu963ra_get_gyro (void)
{
    uint8 dat[6];

    imu963ra_read_acc_gyro_registers(IMU963RA_OUTX_L_G, dat, 6);
    imu963ra_gyro_x = (int16)(((uint16)dat[1] << 8 | dat[0]));
    imu963ra_gyro_y = (int16)(((uint16)dat[3] << 8 | dat[2]));
    imu963ra_gyro_z = (int16)(((uint16)dat[5] << 8 | dat[4]));
}

//-------------------------------------------------------------------------------------------------------------------
// get IMU963RA accelerometer and gyroscope raw data together
//-------------------------------------------------------------------------------------------------------------------
void imu963ra_get_acc_gyro (void)
{
    uint8 dat[12];

    imu963ra_read_acc_gyro_registers(IMU963RA_OUTX_L_G, dat, 12);
    imu963ra_gyro_x = (int16)(((uint16)dat[1]  << 8 | dat[0]));
    imu963ra_gyro_y = (int16)(((uint16)dat[3]  << 8 | dat[2]));
    imu963ra_gyro_z = (int16)(((uint16)dat[5]  << 8 | dat[4]));
    imu963ra_acc_x  = (int16)(((uint16)dat[7]  << 8 | dat[6]));
    imu963ra_acc_y  = (int16)(((uint16)dat[9]  << 8 | dat[8]));
    imu963ra_acc_z  = (int16)(((uint16)dat[11] << 8 | dat[10]));
}


//-------------------------------------------------------------------------------------------------------------------
// ???????    ??? IMU963RA ????????// ??????     void
// ??????     void
// ??????     imu963ra_get_mag();
// ??????     ???????????????????????????
//-------------------------------------------------------------------------------------------------------------------
void imu963ra_get_mag (void)
{
    uint8 temp_status;
    uint8 dat[6];

    imu963ra_write_acc_gyro_register(IMU963RA_FUNC_CFG_ACCESS, 0x40);
    temp_status = imu963ra_read_acc_gyro_register(IMU963RA_STATUS_MASTER);
    if(0x01 & temp_status)
    {
        imu963ra_read_acc_gyro_registers(IMU963RA_SENSOR_HUB_1, dat, 6);
        imu963ra_mag_x = (int16)(((uint16)dat[1] << 8 | dat[0]));
        imu963ra_mag_y = (int16)(((uint16)dat[3] << 8 | dat[2]));
        imu963ra_mag_z = (int16)(((uint16)dat[5] << 8 | dat[4]));
    }
    imu963ra_write_acc_gyro_register(IMU963RA_FUNC_CFG_ACCESS, 0x00);
}

//-------------------------------------------------------------------------------------------------------------------
// ???????    ?????IMU963RA
// ??????     void
// ??????     uint8           1-????????0-????????// ??????     imu963ra_init();
// ??????     
//-------------------------------------------------------------------------------------------------------------------
uint8 imu963ra_init (void)
{
    uint8 return_state = 0;
    system_delay_ms(10);                                                        // ????

#if IMU963RA_USE_SOFT_IIC
    gpio_init(IMU963RA_CS_PIN, GPO, GPIO_LOW, GPO_PUSH_PULL);
    soft_iic_init(&imu963ra_iic_struct, IMU963RA_DEV_ADDR, IMU963RA_SOFT_IIC_DELAY, IMU963RA_SCL_PIN, IMU963RA_SDA_PIN);
#else
    spi_init(IMU963RA_SPI, SPI_MODE0, IMU963RA_SPI_SPEED, IMU963RA_SPC_PIN, IMU963RA_SDI_PIN, IMU963RA_SDO_PIN, SPI_CS_NULL);
    gpio_init(IMU963RA_CS_PIN, GPO, GPIO_LOW, GPO_PUSH_PULL);
#endif

    do
    {
        imu963ra_write_acc_gyro_register(IMU963RA_FUNC_CFG_ACCESS, 0x00);       // ??HUB?????
        imu963ra_write_acc_gyro_register(IMU963RA_CTRL3_C, 0x01);               // ????
        system_delay_ms(2);
        imu963ra_write_acc_gyro_register(IMU963RA_FUNC_CFG_ACCESS, 0x00);       // ??HUB?????
        if(imu963ra_acc_gyro_self_check())
        {
            zf_log(0, "IMU963RA acc and gyro self check error.");
            return_state = 1;
            break;
        }

        imu963ra_write_acc_gyro_register(IMU963RA_INT1_CTRL, 0x03);             // ????? ?????????

        switch(IMU963RA_ACC_SAMPLE_DEFAULT)
        {
            default:
            {
                zf_log(0, "IMU963RA_ACC_SAMPLE_DEFAULT set error.");
                return_state = 1;
            }break;
            case IMU963RA_ACC_SAMPLE_SGN_2G:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL1_XL, 0x50);
                imu963ra_transition_factor[0] = 16393;
            }break;
            case IMU963RA_ACC_SAMPLE_SGN_4G:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL1_XL, 0x58);
                imu963ra_transition_factor[0] = 8197;
            }break;
            case IMU963RA_ACC_SAMPLE_SGN_8G:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL1_XL, 0x5C);
                imu963ra_transition_factor[0] = 4098;
            }break;
            case IMU963RA_ACC_SAMPLE_SGN_16G:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL1_XL, 0x54);
                imu963ra_transition_factor[0] = 2049;
            }break;
        }
        if(1 == return_state)
        {
            break;
        }

        switch(IMU963RA_GYRO_SAMPLE_DEFAULT)
        {
            default:
            {
                zf_log(0, "IMU963RA_GYRO_SAMPLE_DEFAULT set error.");
                return_state = 1;
            }break;
            case IMU963RA_GYRO_SAMPLE_SGN_125DPS:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL2_G, 0x52);
                imu963ra_transition_factor[1] = 228.6;
            }break;
            case IMU963RA_GYRO_SAMPLE_SGN_250DPS:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL2_G, 0x50);
                imu963ra_transition_factor[1] = 114.3;
            }break;
            case IMU963RA_GYRO_SAMPLE_SGN_500DPS:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL2_G, 0x54);
                imu963ra_transition_factor[1] = 57.1;
            }break;
            case IMU963RA_GYRO_SAMPLE_SGN_1000DPS:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL2_G, 0x58);
                imu963ra_transition_factor[1] = 28.6;
            }break;
            case IMU963RA_GYRO_SAMPLE_SGN_2000DPS:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL2_G, 0x5C);
                imu963ra_transition_factor[1] = 14.3;
            }break;
            case IMU963RA_GYRO_SAMPLE_SGN_4000DPS:
            {
                imu963ra_write_acc_gyro_register(IMU963RA_CTRL2_G, 0x51);
                imu963ra_transition_factor[1] = 7.1;
            }break;
        }
        if(1 == return_state)
        {
            break;
        }

        imu963ra_write_acc_gyro_register(IMU963RA_CTRL3_C, 0x44);               // ????????????
        imu963ra_write_acc_gyro_register(IMU963RA_CTRL4_C, 0x02);               // ?????????
        imu963ra_write_acc_gyro_register(IMU963RA_CTRL5_C, 0x00);               // ????????????
        imu963ra_write_acc_gyro_register(IMU963RA_CTRL6_C, 0x00);               // ??????????? ??????? 133hz
        imu963ra_write_acc_gyro_register(IMU963RA_CTRL7_G, 0x00);               // ?????????? ??????
        imu963ra_write_acc_gyro_register(IMU963RA_CTRL9_XL, 0x01);              // ??I3C??

        imu963ra_write_acc_gyro_register(IMU963RA_FUNC_CFG_ACCESS, 0x40);       // ??HUB????? ???????
        imu963ra_write_acc_gyro_register(IMU963RA_MASTER_CONFIG, 0x80);         // ??I2C??
        system_delay_ms(2);
        imu963ra_write_acc_gyro_register(IMU963RA_MASTER_CONFIG, 0x00);         // ??????
        system_delay_ms(2);

        imu963ra_write_mag_register(IMU963RA_MAG_ADDR, IMU963RA_MAG_CONTROL2, 0x80);    // ???????
        system_delay_ms(2);
        imu963ra_write_mag_register(IMU963RA_MAG_ADDR, IMU963RA_MAG_CONTROL2, 0x00);
        system_delay_ms(2);

        if(imu963ra_mag_self_check())
        {
            zf_log(0, "IMU963RA mag self check error.");
            return_state = 1;
            break;
        }

        switch(IMU963RA_MAG_SAMPLE_DEFAULT)
        {
            default:
            {
                zf_log(0, "IMU963RA_MAG_SAMPLE_DEFAULT set error.");
                return_state = 1;
            }break;
            case IMU963RA_MAG_SAMPLE_2G:
            {
                imu963ra_write_mag_register(IMU963RA_MAG_ADDR, IMU963RA_MAG_CONTROL1, 0x09);
                imu963ra_transition_factor[2] = 12000;
            }break;
            case IMU963RA_MAG_SAMPLE_8G:
            {
                imu963ra_write_mag_register(IMU963RA_MAG_ADDR, IMU963RA_MAG_CONTROL1, 0x19);
                imu963ra_transition_factor[2] = 3000;
            }break;
        }
        if(1 == return_state)
        {
            break;
        }

        imu963ra_write_mag_register(IMU963RA_MAG_ADDR, IMU963RA_MAG_FBR, 0x01);
        imu963ra_connect_mag(IMU963RA_MAG_ADDR, IMU963RA_MAG_OUTX_L);

        imu963ra_write_acc_gyro_register(IMU963RA_FUNC_CFG_ACCESS, 0x00);       // ??HUB?????
        system_delay_ms(20);                                                    // ?????????
    }while(0);
    return return_state;
}


