#ifndef __FRIZZ_PACKET_H__
#define __FRIZZ_PACKET_H__

#define MAX_DATA_NUM 10

typedef union {
	unsigned int	w;
	struct {
		unsigned char	num;		///< payload word num
		unsigned char	sen_id;		///< sensor ID
		unsigned char	type;		///< 0x80: SensorOutput, 0x81: Command, 0x82: MessageACK, 0x83: MessageNACK, 0x84: Response, 0x8F: BreakCode
		unsigned char	prefix;		///< 0xFF
	};
} hubhal_format_header_t;
/*! @struct packet_t
 *  @brief  sent data and received data from frizz fw
 */
typedef struct {
    hubhal_format_header_t header;    /*!< frizz fw header */
    unsigned int data[MAX_DATA_NUM];  /*!< sent and received data*/
}packet_t;

#endif
