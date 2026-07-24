/*********************************************************************************************************************
* MSPM0G3507 Opensource Library 鍗筹紙MSPM0G3507 寮€婧愬簱锛夋槸涓€涓熀浜庡畼鏂?SDK 鎺ュ彛鐨勭涓夋柟寮€婧愬簱
* Copyright (c) 2022 SEEKFREE 閫愰绉戞妧
* 
* 鏈枃浠舵槸 MSPM0G3507 寮€婧愬簱鐨勪竴閮ㄥ垎
* 
* MSPM0G3507 寮€婧愬簱 鏄厤璐硅蒋浠?* 鎮ㄥ彲浠ユ牴鎹嚜鐢辫蒋浠跺熀閲戜細鍙戝竷鐨?GPL锛圙NU General Public License锛屽嵆 GNU閫氱敤鍏叡璁稿彲璇侊級鐨勬潯娆?* 鍗?GPL 鐨勭3鐗堬紙鍗?GPL3.0锛夋垨锛堟偍閫夋嫨鐨勶級浠讳綍鍚庢潵鐨勭増鏈紝閲嶆柊鍙戝竷鍜?鎴栦慨鏀瑰畠
* 
* 鏈紑婧愬簱鐨勫彂甯冩槸甯屾湜瀹冭兘鍙戞尌浣滅敤锛屼絾骞舵湭瀵瑰叾浣滀换浣曠殑淇濊瘉
* 鐢氳嚦娌℃湁闅愬惈鐨勯€傞攢鎬ф垨閫傚悎鐗瑰畾鐢ㄩ€旂殑淇濊瘉
* 鏇村缁嗚妭璇峰弬瑙?GPL
* 
* 鎮ㄥ簲璇ュ湪鏀跺埌鏈紑婧愬簱鐨勫悓鏃舵敹鍒颁竴浠?GPL 鐨勫壇鏈?* 濡傛灉娌℃湁锛岃鍙傞槄<https://www.gnu.org/licenses/>
* 
* 棰濆娉ㄦ槑锛?* 鏈紑婧愬簱浣跨敤 GPL3.0 寮€婧愯鍙瘉鍗忚 浠ヤ笂璁稿彲鐢虫槑涓鸿瘧鏂囩増鏈?* 璁稿彲鐢虫槑鑻辨枃鐗堝湪 libraries/doc 鏂囦欢澶逛笅鐨?GPL3_permission_statement.txt 鏂囦欢涓?* 璁稿彲璇佸壇鏈湪 libraries 鏂囦欢澶逛笅 鍗宠鏂囦欢澶逛笅鐨?LICENSE 鏂囦欢
* 娆㈣繋鍚勪綅浣跨敤骞朵紶鎾湰绋嬪簭 浣嗕慨鏀瑰唴瀹规椂蹇呴』淇濈暀閫愰绉戞妧鐨勭増鏉冨０鏄庯紙鍗虫湰澹版槑锛?* 
* 鏂囦欢鍚嶇О          zf_device_imu963ra
* 鍏徃鍚嶇О          鎴愰兘閫愰绉戞妧鏈夐檺鍏徃
* 鐗堟湰淇℃伅          鏌ョ湅 libraries/doc 鏂囦欢澶瑰唴 version 鏂囦欢 鐗堟湰璇存槑
* 寮€鍙戠幆澧?         MDK 5.37
* 閫傜敤骞冲彴          MSPM0G3507
* 搴楅摵閾炬帴          https://seekfree.taobao.com/
* 
* 淇敼璁板綍
* 鏃ユ湡              浣滆€?               澶囨敞
* 2025-06-1        SeekFree            first version
********************************************************************************************************************/
/********************************************************************************************************************
* 鎺ョ嚎瀹氫箟锛?*                  ------------------------------------
*                  妯″潡绠¤剼             鍗曠墖鏈虹鑴?*                  // 纭欢 SPI 寮曡剼
*                  SCL/SPC            鏌ョ湅 zf_device_imu963ra.h 涓?IMU963RA_SPC_PIN 瀹忓畾涔?*                  SDA/DSI            鏌ョ湅 zf_device_imu963ra.h 涓?IMU963RA_SDI_PIN 瀹忓畾涔?*                  SA0/SDO            鏌ョ湅 zf_device_imu963ra.h 涓?IMU963RA_SDO_PIN 瀹忓畾涔?*                  CS                 鏌ョ湅 zf_device_imu963ra.h 涓?IMU963RA_CS_PIN  瀹忓畾涔?*                  VCC                3.3V鐢垫簮
*                  GND                鐢垫簮鍦?*                  鍏朵綑寮曡剼鎮┖
*
*                  // 杞欢 IIC 寮曡剼
*                  SCL/SPC            鏌ョ湅 zf_device_imu963ra.h 涓?IMU963RA_SCL_PIN 瀹忓畾涔?*                  SDA/DSI            鏌ョ湅 zf_device_imu963ra.h 涓?IMU963RA_SDA_PIN 瀹忓畾涔?*                  VCC                3.3V鐢垫簮
*                  GND                鐢垫簮鍦?*                  鍏朵綑寮曡剼鎮┖
*                  ------------------------------------
********************************************************************************************************************/

#ifndef _zf_device_imu963ra_h_
#define _zf_device_imu963ra_h_

#include "zf_common_typedef.h"

//================================================瀹氫箟 IMU963RA 鍩烘湰閰嶇疆================================================
#define IMU963RA_USE_SOFT_IIC                       (0)                         // 榛樿浣跨敤纭欢 SPI 鏂瑰紡椹卞姩
#if IMU963RA_USE_SOFT_IIC                                                 		// 杩欎袱娈?棰滆壊姝ｅ父鐨勬墠鏄纭殑 棰滆壊鐏扮殑灏辨槸娌℃湁鐢ㄧ殑
//====================================================杞欢 IIC 椹卞姩====================================================
//====================================================娉ㄦ剰 娉ㄦ剰 娉ㄦ剰   浣跨敤IIC妯″紡涓嬩笉瑕佹帴CS
//====================================================娉ㄦ剰 娉ㄦ剰 娉ㄦ剰   浣跨敤IIC妯″紡涓嬩笉瑕佹帴CS
//====================================================娉ㄦ剰 娉ㄦ剰 娉ㄦ剰   浣跨敤IIC妯″紡涓嬩笉瑕佹帴CS
#define IMU963RA_SOFT_IIC_DELAY                     ( 100  )                	// 杞欢 IIC 鐨勬椂閽熷欢鏃跺懆鏈?鏁板€艰秺灏?IIC 閫氫俊閫熺巼瓒婂揩
#define IMU963RA_SCL_PIN                            ( B23 )                    	// 杞欢 IIC SCL 寮曡剼 杩炴帴 IMU963RA 鐨?SCL 寮曡剼
#define IMU963RA_SDA_PIN                            ( B22 )                    	// 杞欢 IIC SDA 寮曡剼 杩炴帴 IMU963RA 鐨?SDA 寮曡剼
//====================================================杞欢 IIC 椹卞姩====================================================
#else

//====================================================纭欢 SPI 椹卞姩====================================================
#define IMU963RA_SPI_SPEED                           ( 8 * 1000 * 1000 )      	// 纭欢 SPI 閫熺巼
#define IMU963RA_SPI                                 ( SPI_1         )     		// 纭欢 SPI 鍙?
#define IMU963RA_SPC_PIN                             ( SPI1_SCK_B23  )     		// 纭欢 SPI SCK 寮曡剼
#define IMU963RA_SDI_PIN                             ( SPI1_MOSI_B22 )        	// 纭欢 SPI MOSI 寮曡剼
#define IMU963RA_SDO_PIN                             ( SPI1_MISO_B21 )     		// 纭欢 SPI MISO 寮曡剼
//====================================================纭欢 SPI 椹卞姩====================================================
#endif

#define IMU963RA_CS_PIN                              ( B19 )                   	// CS 鐗囬€夊紩鑴?
#define IMU963RA_CS(x)                               ( (x) ? (gpio_high(IMU963RA_CS_PIN)) : (gpio_low(IMU963RA_CS_PIN)) )

typedef enum
{
    IMU963RA_ACC_SAMPLE_SGN_2G ,                                                // 鍔犻€熷害璁￠噺绋?卤2G  (ACC = Accelerometer 鍔犻€熷害璁? (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (G = g 閲嶅姏鍔犻€熷害 g鈮?.80 m/s^2)
    IMU963RA_ACC_SAMPLE_SGN_4G ,                                                // 鍔犻€熷害璁￠噺绋?卤4G  (ACC = Accelerometer 鍔犻€熷害璁? (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (G = g 閲嶅姏鍔犻€熷害 g鈮?.80 m/s^2)
    IMU963RA_ACC_SAMPLE_SGN_8G ,                                                // 鍔犻€熷害璁￠噺绋?卤8G  (ACC = Accelerometer 鍔犻€熷害璁? (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (G = g 閲嶅姏鍔犻€熷害 g鈮?.80 m/s^2)
    IMU963RA_ACC_SAMPLE_SGN_16G,                                                // 鍔犻€熷害璁￠噺绋?卤16G (ACC = Accelerometer 鍔犻€熷害璁? (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (G = g 閲嶅姏鍔犻€熷害 g鈮?.80 m/s^2)
}imu963ra_acc_sample_config;

typedef enum
{
    IMU963RA_GYRO_SAMPLE_SGN_125DPS ,                                           // 闄€铻轰华閲忕▼ 卤125DPS  (GYRO = Gyroscope 闄€铻轰华) (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (DPS = Degree Per Second 瑙掗€熷害鍗曚綅 掳/S)
    IMU963RA_GYRO_SAMPLE_SGN_250DPS ,                                           // 闄€铻轰华閲忕▼ 卤250DPS  (GYRO = Gyroscope 闄€铻轰华) (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (DPS = Degree Per Second 瑙掗€熷害鍗曚綅 掳/S)
    IMU963RA_GYRO_SAMPLE_SGN_500DPS ,                                           // 闄€铻轰华閲忕▼ 卤500DPS  (GYRO = Gyroscope 闄€铻轰华) (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (DPS = Degree Per Second 瑙掗€熷害鍗曚綅 掳/S)
    IMU963RA_GYRO_SAMPLE_SGN_1000DPS,                                           // 闄€铻轰华閲忕▼ 卤1000DPS (GYRO = Gyroscope 闄€铻轰华) (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (DPS = Degree Per Second 瑙掗€熷害鍗曚綅 掳/S)
    IMU963RA_GYRO_SAMPLE_SGN_2000DPS,                                           // 闄€铻轰华閲忕▼ 卤2000DPS (GYRO = Gyroscope 闄€铻轰华) (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (DPS = Degree Per Second 瑙掗€熷害鍗曚綅 掳/S)
    IMU963RA_GYRO_SAMPLE_SGN_4000DPS,                                           // 闄€铻轰华閲忕▼ 卤4000DPS (GYRO = Gyroscope 闄€铻轰华) (SGN = signum 甯︾鍙锋暟 琛ㄧず姝ｈ礋鑼冨洿) (DPS = Degree Per Second 瑙掗€熷害鍗曚綅 掳/S)
}imu963ra_gyro_sample_config;

typedef enum
{
    IMU963RA_MAG_SAMPLE_2G,                                                     // 纾佸姏璁￠噺绋?2G (MAG = Magnetometer 纾佸姏璁? (G = Gs 楂樻柉)
    IMU963RA_MAG_SAMPLE_8G,                                                     // 纾佸姏璁￠噺绋?8G (MAG = Magnetometer 纾佸姏璁? (G = Gs 楂樻柉)
}imu963ra_mag_sample_config;

#define IMU963RA_ACC_SAMPLE_DEFAULT                 ( IMU963RA_ACC_SAMPLE_SGN_8G )        // 鍦ㄨ繖璁剧疆榛樿鐨?鍔犻€熷害璁?鍒濆鍖栭噺绋?
#define IMU963RA_GYRO_SAMPLE_DEFAULT                ( IMU963RA_GYRO_SAMPLE_SGN_2000DPS )  // 鍦ㄨ繖璁剧疆榛樿鐨?闄€铻轰华   鍒濆鍖栭噺绋?
#define IMU963RA_MAG_SAMPLE_DEFAULT                 ( IMU963RA_MAG_SAMPLE_8G )            // 鍦ㄨ繖璁剧疆榛樿鐨?纾佸姏璁?  鍒濆鍖栭噺绋?
#define IMU963RA_TIMEOUT_COUNT                      ( 0x00FF )                            // IMU963RA 瓒呮椂璁℃暟
//================================================瀹氫箟 IMU963RA 鍩烘湰閰嶇疆================================================


//================================================瀹氫箟 IMU963RA 鍐呴儴鍦板潃================================================
#define IMU963RA_DEV_ADDR                           (0x6B)                      // SA0鎺ュ湴锛?x6A SA0涓婃媺锛?x6B 妯″潡榛樿涓婃媺
#define IMU963RA_SPI_W                              (0x00)
#define IMU963RA_SPI_R                              (0x80)

#define IMU963RA_FUNC_CFG_ACCESS                    (0x01)
#define IMU963RA_INT1_CTRL                          (0x0D)
#define IMU963RA_WHO_AM_I                           (0x0F)
#define IMU963RA_CTRL1_XL                           (0x10)
#define IMU963RA_CTRL2_G                            (0x11)
#define IMU963RA_CTRL3_C                            (0x12)
#define IMU963RA_CTRL4_C                            (0x13)
#define IMU963RA_CTRL5_C                            (0x14)
#define IMU963RA_CTRL6_C                            (0x15)
#define IMU963RA_CTRL7_G                            (0x16)
#define IMU963RA_CTRL9_XL                           (0x18)
#define IMU963RA_OUTX_L_G                           (0x22)
#define IMU963RA_OUTX_L_A                           (0x28)

//闆嗙嚎鍣ㄥ姛鑳界浉鍏冲瘎瀛樺櫒 闇€瑕佸皢FUNC_CFG_ACCESS鐨凷HUB_REG_ACCESS浣嶈缃负1鎵嶈兘姝ｇ‘璁块棶
#define IMU963RA_SENSOR_HUB_1                       (0x02)
#define IMU963RA_MASTER_CONFIG                      (0x14)
#define IMU963RA_SLV0_ADD                           (0x15)
#define IMU963RA_SLV0_SUBADD                        (0x16)
#define IMU963RA_SLV0_CONFIG                        (0x17)
#define IMU963RA_DATAWRITE_SLV0                     (0x21)
#define IMU963RA_STATUS_MASTER                      (0x22)

#define IMU963RA_MAG_ADDR                           (0x0D)                      // 7浣岻IC鍦板潃
#define IMU963RA_MAG_OUTX_L                         (0x00)
#define IMU963RA_MAG_CONTROL1                       (0x09)
#define IMU963RA_MAG_CONTROL2                       (0x0A)
#define IMU963RA_MAG_FBR                            (0x0B)
#define IMU963RA_MAG_CHIP_ID                        (0x0D)

#define IMU963RA_ACC_SAMPLE                         (0x3C)                      // 鍔犻€熷害璁￠噺绋?// 璁剧疆涓?0x30 鍔犻€熷害閲忕▼涓?卤2G      鑾峰彇鍒扮殑鍔犻€熷害璁℃暟鎹?闄や互16393锛屽彲浠ヨ浆鍖栦负甯︾墿鐞嗗崟浣嶇殑鏁版嵁锛屽崟浣嶏細g(m/s^2)
// 璁剧疆涓?0x38 鍔犻€熷害閲忕▼涓?卤4G      鑾峰彇鍒扮殑鍔犻€熷害璁℃暟鎹?闄や互8197锛?鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅锛歡(m/s^2)
// 璁剧疆涓?0x3C 鍔犻€熷害閲忕▼涓?卤8G      鑾峰彇鍒扮殑鍔犻€熷害璁℃暟鎹?闄や互4098锛?鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅锛歡(m/s^2)
// 璁剧疆涓?0x34 鍔犻€熷害閲忕▼涓?卤16G     鑾峰彇鍒扮殑鍔犻€熷害璁℃暟鎹?闄や互2049锛?鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅锛歡(m/s^2)

#define IMU963RA_GYR_SAMPLE                         (0x5C)                      // 闄€铻轰华閲忕▼
// 璁剧疆涓?0x52 闄€铻轰华閲忕▼涓?卤125dps  鑾峰彇鍒扮殑闄€铻轰华鏁版嵁闄や互228.6锛?  鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅涓猴細掳/s
// 璁剧疆涓?0x50 闄€铻轰华閲忕▼涓?卤250dps  鑾峰彇鍒扮殑闄€铻轰华鏁版嵁闄や互114.3锛?  鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅涓猴細掳/s
// 璁剧疆涓?0x54 闄€铻轰华閲忕▼涓?卤500dps  鑾峰彇鍒扮殑闄€铻轰华鏁版嵁闄や互57.1锛?   鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅涓猴細掳/s
// 璁剧疆涓?0x58 闄€铻轰华閲忕▼涓?卤1000dps 鑾峰彇鍒扮殑闄€铻轰华鏁版嵁闄や互28.6锛?   鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅涓猴細掳/s
// 璁剧疆涓?0x5C 闄€铻轰华閲忕▼涓?卤2000dps 鑾峰彇鍒扮殑闄€铻轰华鏁版嵁闄や互14.3锛?   鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅涓猴細掳/s
// 璁剧疆涓?0x51 闄€铻轰华閲忕▼涓?卤4000dps 鑾峰彇鍒扮殑闄€铻轰华鏁版嵁闄や互7.1锛?    鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅涓猴細掳/s

#define IMU963RA_MAG_SAMPLE                         (0x19)                      // 鍦扮璁￠噺绋?// 璁剧疆涓?0x19 纾佸姏璁￠噺绋嬩负:8G     鑾峰彇鍒扮殑鍔犻€熷害璁℃暟鎹?闄や互3000锛?鍙互杞寲涓哄甫鐗╃悊鍗曚綅鐨勬暟鎹紝鍗曚綅锛欸(楂樻柉)
// 璁剧疆涓?0x09 纾佸姏璁￠噺绋嬩负:2G     鑾峰彇鍒扮殑鍔犻€熷害璁℃暟鎹?闄や互12000锛屽彲浠ヨ浆鍖栦负甯︾墿鐞嗗崟浣嶇殑鏁版嵁锛屽崟浣嶏細G(楂樻柉)
//================================================瀹氫箟 IMU963RA 鍐呴儴鍦板潃================================================


//================================================澹版槑 IMU963RA 鍏ㄥ眬鍙橀噺================================================
extern int16 imu963ra_acc_x,  imu963ra_acc_y,  imu963ra_acc_z;                  // 涓夎酱闄€铻轰华鏁版嵁      GYRO (闄€铻轰华)
extern int16 imu963ra_gyro_x, imu963ra_gyro_y, imu963ra_gyro_z;                 // 涓夎酱鍔犻€熷害璁℃暟鎹?    ACC  (accelerometer 鍔犻€熷害璁?
extern int16 imu963ra_mag_x,  imu963ra_mag_y,  imu963ra_mag_z;                  // 涓夎酱纾佸姏璁℃暟鎹?     MAG  (magnetometer 纾佸姏璁?
extern float imu963ra_transition_factor[3];                                     // 杞崲瀹為檯鍊肩殑姣斾緥
//================================================澹版槑 IMU963RA 鍏ㄥ眬鍙橀噺================================================


//================================================澹版槑 IMU963RA 鍩虹鍑芥暟================================================
//================================================Basic IMU963RA function declarations=============================================
void    imu963ra_get_acc            (void);                                     // get IMU963RA accelerometer raw data
void    imu963ra_get_gyro           (void);                                     // get IMU963RA gyroscope raw data
void    imu963ra_get_acc_gyro       (void);                                     // get IMU963RA accelerometer and gyroscope raw data together
void    imu963ra_get_mag            (void);                                     // get IMU963RA magnetometer raw data
uint8   imu963ra_init               (void);                                     // initialize IMU963RA
//================================================Basic IMU963RA function declarations=============================================
//================================================澹版槑 IMU963RA 鎷撳睍鍑芥暟================================================
//-------------------------------------------------------------------------------------------------------------------
// 鍑芥暟绠€浠?    灏?IMU963RA 鍔犻€熷害璁℃暟鎹浆鎹负瀹為檯鐗╃悊鏁版嵁
// 鍙傛暟璇存槑     acc_value       浠绘剰杞寸殑鍔犻€熷害璁℃暟鎹?// 杩斿洖鍙傛暟     void
// 浣跨敤绀轰緥     float data = imu963ra_acc_transition(imu963ra_acc_x);               // 鍗曚綅涓?g(m/s^2)
// 澶囨敞淇℃伅
//-------------------------------------------------------------------------------------------------------------------
#define imu963ra_acc_transition(acc_value)      ((float)acc_value / imu963ra_transition_factor[0])

//-------------------------------------------------------------------------------------------------------------------
// 鍑芥暟绠€浠?    灏?IMU963RA 闄€铻轰华鏁版嵁杞崲涓哄疄闄呯墿鐞嗘暟鎹?// 鍙傛暟璇存槑     gyro_value      浠绘剰杞寸殑闄€铻轰华鏁版嵁
// 杩斿洖鍙傛暟     void
// 浣跨敤绀轰緥     float data = imu963ra_gyro_transition(imu963ra_gyro_x);             // 鍗曚綅涓?掳/s
// 澶囨敞淇℃伅
//-------------------------------------------------------------------------------------------------------------------
#define imu963ra_gyro_transition(gyro_value)    ((float)gyro_value / imu963ra_transition_factor[1])

//-------------------------------------------------------------------------------------------------------------------
// 鍑芥暟绠€浠?    灏?IMU963RA 纾佸姏璁℃暟鎹浆鎹负瀹為檯鐗╃悊鏁版嵁
// 鍙傛暟璇存槑     mag_value       浠绘剰杞寸殑纾佸姏璁℃暟鎹?// 杩斿洖鍙傛暟     void
// 浣跨敤绀轰緥     float data = imu963ra_mag_transition(imu963ra_mag_x);               // 鍗曚綅涓?G
// 澶囨敞淇℃伅
//-------------------------------------------------------------------------------------------------------------------
#define imu963ra_mag_transition(mag_value)    ((float)mag_value / imu963ra_transition_factor[2])
//================================================澹版槑 IMU963RA 鎷撳睍鍑芥暟================================================

#endif

