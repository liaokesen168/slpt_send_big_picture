/*
 *  Copyright (C) 2014 Ingenic Semiconductor
 *
 *  SunWenZhong(Fighter) <wenzhong.sun@ingenic.com, wanmyqawdr@126.com>
 *
 *  Elf/IDWS Project
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *
 * ======================================================
 *  !!! I say: This implementation borrow many codes !!!
 *  !!!        from linux project                    !!!
 *  !!!                                              !!!
 *  !!!        Thank you for their excellent work.   !!!
 * ======================================================
 *
 *
 */


#include <iwds.h>

#include <utils/crc32.h>


# if __BYTE_ORDER == __LITTLE_ENDIAN
#   define tole(x) (x)
#   define tobe(x) iwds_bswap_32(x)
# elif __BYTE_ORDER == __BIG_ENDIAN
#   define tole(x) iwds_bswap_32(x)
#   define tobe(x) (x)
#endif

namespace Iwds
{

    static u32 crc32table_le[8][256] = {{
    tole(0x00000000L), tole(0x77073096L), tole(0xee0e612cL), tole(0x990951baL),
    tole(0x076dc419L), tole(0x706af48fL), tole(0xe963a535L), tole(0x9e6495a3L),
    tole(0x0edb8832L), tole(0x79dcb8a4L), tole(0xe0d5e91eL), tole(0x97d2d988L),
    tole(0x09b64c2bL), tole(0x7eb17cbdL), tole(0xe7b82d07L), tole(0x90bf1d91L),
    tole(0x1db71064L), tole(0x6ab020f2L), tole(0xf3b97148L), tole(0x84be41deL),
    tole(0x1adad47dL), tole(0x6ddde4ebL), tole(0xf4d4b551L), tole(0x83d385c7L),
    tole(0x136c9856L), tole(0x646ba8c0L), tole(0xfd62f97aL), tole(0x8a65c9ecL),
    tole(0x14015c4fL), tole(0x63066cd9L), tole(0xfa0f3d63L), tole(0x8d080df5L),
    tole(0x3b6e20c8L), tole(0x4c69105eL), tole(0xd56041e4L), tole(0xa2677172L),
    tole(0x3c03e4d1L), tole(0x4b04d447L), tole(0xd20d85fdL), tole(0xa50ab56bL),
    tole(0x35b5a8faL), tole(0x42b2986cL), tole(0xdbbbc9d6L), tole(0xacbcf940L),
    tole(0x32d86ce3L), tole(0x45df5c75L), tole(0xdcd60dcfL), tole(0xabd13d59L),
    tole(0x26d930acL), tole(0x51de003aL), tole(0xc8d75180L), tole(0xbfd06116L),
    tole(0x21b4f4b5L), tole(0x56b3c423L), tole(0xcfba9599L), tole(0xb8bda50fL),
    tole(0x2802b89eL), tole(0x5f058808L), tole(0xc60cd9b2L), tole(0xb10be924L),
    tole(0x2f6f7c87L), tole(0x58684c11L), tole(0xc1611dabL), tole(0xb6662d3dL),
    tole(0x76dc4190L), tole(0x01db7106L), tole(0x98d220bcL), tole(0xefd5102aL),
    tole(0x71b18589L), tole(0x06b6b51fL), tole(0x9fbfe4a5L), tole(0xe8b8d433L),
    tole(0x7807c9a2L), tole(0x0f00f934L), tole(0x9609a88eL), tole(0xe10e9818L),
    tole(0x7f6a0dbbL), tole(0x086d3d2dL), tole(0x91646c97L), tole(0xe6635c01L),
    tole(0x6b6b51f4L), tole(0x1c6c6162L), tole(0x856530d8L), tole(0xf262004eL),
    tole(0x6c0695edL), tole(0x1b01a57bL), tole(0x8208f4c1L), tole(0xf50fc457L),
    tole(0x65b0d9c6L), tole(0x12b7e950L), tole(0x8bbeb8eaL), tole(0xfcb9887cL),
    tole(0x62dd1ddfL), tole(0x15da2d49L), tole(0x8cd37cf3L), tole(0xfbd44c65L),
    tole(0x4db26158L), tole(0x3ab551ceL), tole(0xa3bc0074L), tole(0xd4bb30e2L),
    tole(0x4adfa541L), tole(0x3dd895d7L), tole(0xa4d1c46dL), tole(0xd3d6f4fbL),
    tole(0x4369e96aL), tole(0x346ed9fcL), tole(0xad678846L), tole(0xda60b8d0L),
    tole(0x44042d73L), tole(0x33031de5L), tole(0xaa0a4c5fL), tole(0xdd0d7cc9L),
    tole(0x5005713cL), tole(0x270241aaL), tole(0xbe0b1010L), tole(0xc90c2086L),
    tole(0x5768b525L), tole(0x206f85b3L), tole(0xb966d409L), tole(0xce61e49fL),
    tole(0x5edef90eL), tole(0x29d9c998L), tole(0xb0d09822L), tole(0xc7d7a8b4L),
    tole(0x59b33d17L), tole(0x2eb40d81L), tole(0xb7bd5c3bL), tole(0xc0ba6cadL),
    tole(0xedb88320L), tole(0x9abfb3b6L), tole(0x03b6e20cL), tole(0x74b1d29aL),
    tole(0xead54739L), tole(0x9dd277afL), tole(0x04db2615L), tole(0x73dc1683L),
    tole(0xe3630b12L), tole(0x94643b84L), tole(0x0d6d6a3eL), tole(0x7a6a5aa8L),
    tole(0xe40ecf0bL), tole(0x9309ff9dL), tole(0x0a00ae27L), tole(0x7d079eb1L),
    tole(0xf00f9344L), tole(0x8708a3d2L), tole(0x1e01f268L), tole(0x6906c2feL),
    tole(0xf762575dL), tole(0x806567cbL), tole(0x196c3671L), tole(0x6e6b06e7L),
    tole(0xfed41b76L), tole(0x89d32be0L), tole(0x10da7a5aL), tole(0x67dd4accL),
    tole(0xf9b9df6fL), tole(0x8ebeeff9L), tole(0x17b7be43L), tole(0x60b08ed5L),
    tole(0xd6d6a3e8L), tole(0xa1d1937eL), tole(0x38d8c2c4L), tole(0x4fdff252L),
    tole(0xd1bb67f1L), tole(0xa6bc5767L), tole(0x3fb506ddL), tole(0x48b2364bL),
    tole(0xd80d2bdaL), tole(0xaf0a1b4cL), tole(0x36034af6L), tole(0x41047a60L),
    tole(0xdf60efc3L), tole(0xa867df55L), tole(0x316e8eefL), tole(0x4669be79L),
    tole(0xcb61b38cL), tole(0xbc66831aL), tole(0x256fd2a0L), tole(0x5268e236L),
    tole(0xcc0c7795L), tole(0xbb0b4703L), tole(0x220216b9L), tole(0x5505262fL),
    tole(0xc5ba3bbeL), tole(0xb2bd0b28L), tole(0x2bb45a92L), tole(0x5cb36a04L),
    tole(0xc2d7ffa7L), tole(0xb5d0cf31L), tole(0x2cd99e8bL), tole(0x5bdeae1dL),
    tole(0x9b64c2b0L), tole(0xec63f226L), tole(0x756aa39cL), tole(0x026d930aL),
    tole(0x9c0906a9L), tole(0xeb0e363fL), tole(0x72076785L), tole(0x05005713L),
    tole(0x95bf4a82L), tole(0xe2b87a14L), tole(0x7bb12baeL), tole(0x0cb61b38L),
    tole(0x92d28e9bL), tole(0xe5d5be0dL), tole(0x7cdcefb7L), tole(0x0bdbdf21L),
    tole(0x86d3d2d4L), tole(0xf1d4e242L), tole(0x68ddb3f8L), tole(0x1fda836eL),
    tole(0x81be16cdL), tole(0xf6b9265bL), tole(0x6fb077e1L), tole(0x18b74777L),
    tole(0x88085ae6L), tole(0xff0f6a70L), tole(0x66063bcaL), tole(0x11010b5cL),
    tole(0x8f659effL), tole(0xf862ae69L), tole(0x616bffd3L), tole(0x166ccf45L),
    tole(0xa00ae278L), tole(0xd70dd2eeL), tole(0x4e048354L), tole(0x3903b3c2L),
    tole(0xa7672661L), tole(0xd06016f7L), tole(0x4969474dL), tole(0x3e6e77dbL),
    tole(0xaed16a4aL), tole(0xd9d65adcL), tole(0x40df0b66L), tole(0x37d83bf0L),
    tole(0xa9bcae53L), tole(0xdebb9ec5L), tole(0x47b2cf7fL), tole(0x30b5ffe9L),
    tole(0xbdbdf21cL), tole(0xcabac28aL), tole(0x53b39330L), tole(0x24b4a3a6L),
    tole(0xbad03605L), tole(0xcdd70693L), tole(0x54de5729L), tole(0x23d967bfL),
    tole(0xb3667a2eL), tole(0xc4614ab8L), tole(0x5d681b02L), tole(0x2a6f2b94L),
    tole(0xb40bbe37L), tole(0xc30c8ea1L), tole(0x5a05df1bL), tole(0x2d02ef8dL)},
    {
    tole(0x00000000L), tole(0x191b3141L), tole(0x32366282L), tole(0x2b2d53c3L),
    tole(0x646cc504L), tole(0x7d77f445L), tole(0x565aa786L), tole(0x4f4196c7L),
    tole(0xc8d98a08L), tole(0xd1c2bb49L), tole(0xfaefe88aL), tole(0xe3f4d9cbL),
    tole(0xacb54f0cL), tole(0xb5ae7e4dL), tole(0x9e832d8eL), tole(0x87981ccfL),
    tole(0x4ac21251L), tole(0x53d92310L), tole(0x78f470d3L), tole(0x61ef4192L),
    tole(0x2eaed755L), tole(0x37b5e614L), tole(0x1c98b5d7L), tole(0x05838496L),
    tole(0x821b9859L), tole(0x9b00a918L), tole(0xb02dfadbL), tole(0xa936cb9aL),
    tole(0xe6775d5dL), tole(0xff6c6c1cL), tole(0xd4413fdfL), tole(0xcd5a0e9eL),
    tole(0x958424a2L), tole(0x8c9f15e3L), tole(0xa7b24620L), tole(0xbea97761L),
    tole(0xf1e8e1a6L), tole(0xe8f3d0e7L), tole(0xc3de8324L), tole(0xdac5b265L),
    tole(0x5d5daeaaL), tole(0x44469febL), tole(0x6f6bcc28L), tole(0x7670fd69L),
    tole(0x39316baeL), tole(0x202a5aefL), tole(0x0b07092cL), tole(0x121c386dL),
    tole(0xdf4636f3L), tole(0xc65d07b2L), tole(0xed705471L), tole(0xf46b6530L),
    tole(0xbb2af3f7L), tole(0xa231c2b6L), tole(0x891c9175L), tole(0x9007a034L),
    tole(0x179fbcfbL), tole(0x0e848dbaL), tole(0x25a9de79L), tole(0x3cb2ef38L),
    tole(0x73f379ffL), tole(0x6ae848beL), tole(0x41c51b7dL), tole(0x58de2a3cL),
    tole(0xf0794f05L), tole(0xe9627e44L), tole(0xc24f2d87L), tole(0xdb541cc6L),
    tole(0x94158a01L), tole(0x8d0ebb40L), tole(0xa623e883L), tole(0xbf38d9c2L),
    tole(0x38a0c50dL), tole(0x21bbf44cL), tole(0x0a96a78fL), tole(0x138d96ceL),
    tole(0x5ccc0009L), tole(0x45d73148L), tole(0x6efa628bL), tole(0x77e153caL),
    tole(0xbabb5d54L), tole(0xa3a06c15L), tole(0x888d3fd6L), tole(0x91960e97L),
    tole(0xded79850L), tole(0xc7cca911L), tole(0xece1fad2L), tole(0xf5facb93L),
    tole(0x7262d75cL), tole(0x6b79e61dL), tole(0x4054b5deL), tole(0x594f849fL),
    tole(0x160e1258L), tole(0x0f152319L), tole(0x243870daL), tole(0x3d23419bL),
    tole(0x65fd6ba7L), tole(0x7ce65ae6L), tole(0x57cb0925L), tole(0x4ed03864L),
    tole(0x0191aea3L), tole(0x188a9fe2L), tole(0x33a7cc21L), tole(0x2abcfd60L),
    tole(0xad24e1afL), tole(0xb43fd0eeL), tole(0x9f12832dL), tole(0x8609b26cL),
    tole(0xc94824abL), tole(0xd05315eaL), tole(0xfb7e4629L), tole(0xe2657768L),
    tole(0x2f3f79f6L), tole(0x362448b7L), tole(0x1d091b74L), tole(0x04122a35L),
    tole(0x4b53bcf2L), tole(0x52488db3L), tole(0x7965de70L), tole(0x607eef31L),
    tole(0xe7e6f3feL), tole(0xfefdc2bfL), tole(0xd5d0917cL), tole(0xcccba03dL),
    tole(0x838a36faL), tole(0x9a9107bbL), tole(0xb1bc5478L), tole(0xa8a76539L),
    tole(0x3b83984bL), tole(0x2298a90aL), tole(0x09b5fac9L), tole(0x10aecb88L),
    tole(0x5fef5d4fL), tole(0x46f46c0eL), tole(0x6dd93fcdL), tole(0x74c20e8cL),
    tole(0xf35a1243L), tole(0xea412302L), tole(0xc16c70c1L), tole(0xd8774180L),
    tole(0x9736d747L), tole(0x8e2de606L), tole(0xa500b5c5L), tole(0xbc1b8484L),
    tole(0x71418a1aL), tole(0x685abb5bL), tole(0x4377e898L), tole(0x5a6cd9d9L),
    tole(0x152d4f1eL), tole(0x0c367e5fL), tole(0x271b2d9cL), tole(0x3e001cddL),
    tole(0xb9980012L), tole(0xa0833153L), tole(0x8bae6290L), tole(0x92b553d1L),
    tole(0xddf4c516L), tole(0xc4eff457L), tole(0xefc2a794L), tole(0xf6d996d5L),
    tole(0xae07bce9L), tole(0xb71c8da8L), tole(0x9c31de6bL), tole(0x852aef2aL),
    tole(0xca6b79edL), tole(0xd37048acL), tole(0xf85d1b6fL), tole(0xe1462a2eL),
    tole(0x66de36e1L), tole(0x7fc507a0L), tole(0x54e85463L), tole(0x4df36522L),
    tole(0x02b2f3e5L), tole(0x1ba9c2a4L), tole(0x30849167L), tole(0x299fa026L),
    tole(0xe4c5aeb8L), tole(0xfdde9ff9L), tole(0xd6f3cc3aL), tole(0xcfe8fd7bL),
    tole(0x80a96bbcL), tole(0x99b25afdL), tole(0xb29f093eL), tole(0xab84387fL),
    tole(0x2c1c24b0L), tole(0x350715f1L), tole(0x1e2a4632L), tole(0x07317773L),
    tole(0x4870e1b4L), tole(0x516bd0f5L), tole(0x7a468336L), tole(0x635db277L),
    tole(0xcbfad74eL), tole(0xd2e1e60fL), tole(0xf9ccb5ccL), tole(0xe0d7848dL),
    tole(0xaf96124aL), tole(0xb68d230bL), tole(0x9da070c8L), tole(0x84bb4189L),
    tole(0x03235d46L), tole(0x1a386c07L), tole(0x31153fc4L), tole(0x280e0e85L),
    tole(0x674f9842L), tole(0x7e54a903L), tole(0x5579fac0L), tole(0x4c62cb81L),
    tole(0x8138c51fL), tole(0x9823f45eL), tole(0xb30ea79dL), tole(0xaa1596dcL),
    tole(0xe554001bL), tole(0xfc4f315aL), tole(0xd7626299L), tole(0xce7953d8L),
    tole(0x49e14f17L), tole(0x50fa7e56L), tole(0x7bd72d95L), tole(0x62cc1cd4L),
    tole(0x2d8d8a13L), tole(0x3496bb52L), tole(0x1fbbe891L), tole(0x06a0d9d0L),
    tole(0x5e7ef3ecL), tole(0x4765c2adL), tole(0x6c48916eL), tole(0x7553a02fL),
    tole(0x3a1236e8L), tole(0x230907a9L), tole(0x0824546aL), tole(0x113f652bL),
    tole(0x96a779e4L), tole(0x8fbc48a5L), tole(0xa4911b66L), tole(0xbd8a2a27L),
    tole(0xf2cbbce0L), tole(0xebd08da1L), tole(0xc0fdde62L), tole(0xd9e6ef23L),
    tole(0x14bce1bdL), tole(0x0da7d0fcL), tole(0x268a833fL), tole(0x3f91b27eL),
    tole(0x70d024b9L), tole(0x69cb15f8L), tole(0x42e6463bL), tole(0x5bfd777aL),
    tole(0xdc656bb5L), tole(0xc57e5af4L), tole(0xee530937L), tole(0xf7483876L),
    tole(0xb809aeb1L), tole(0xa1129ff0L), tole(0x8a3fcc33L), tole(0x9324fd72L)},
    {
    tole(0x00000000L), tole(0x01c26a37L), tole(0x0384d46eL), tole(0x0246be59L),
    tole(0x0709a8dcL), tole(0x06cbc2ebL), tole(0x048d7cb2L), tole(0x054f1685L),
    tole(0x0e1351b8L), tole(0x0fd13b8fL), tole(0x0d9785d6L), tole(0x0c55efe1L),
    tole(0x091af964L), tole(0x08d89353L), tole(0x0a9e2d0aL), tole(0x0b5c473dL),
    tole(0x1c26a370L), tole(0x1de4c947L), tole(0x1fa2771eL), tole(0x1e601d29L),
    tole(0x1b2f0bacL), tole(0x1aed619bL), tole(0x18abdfc2L), tole(0x1969b5f5L),
    tole(0x1235f2c8L), tole(0x13f798ffL), tole(0x11b126a6L), tole(0x10734c91L),
    tole(0x153c5a14L), tole(0x14fe3023L), tole(0x16b88e7aL), tole(0x177ae44dL),
    tole(0x384d46e0L), tole(0x398f2cd7L), tole(0x3bc9928eL), tole(0x3a0bf8b9L),
    tole(0x3f44ee3cL), tole(0x3e86840bL), tole(0x3cc03a52L), tole(0x3d025065L),
    tole(0x365e1758L), tole(0x379c7d6fL), tole(0x35dac336L), tole(0x3418a901L),
    tole(0x3157bf84L), tole(0x3095d5b3L), tole(0x32d36beaL), tole(0x331101ddL),
    tole(0x246be590L), tole(0x25a98fa7L), tole(0x27ef31feL), tole(0x262d5bc9L),
    tole(0x23624d4cL), tole(0x22a0277bL), tole(0x20e69922L), tole(0x2124f315L),
    tole(0x2a78b428L), tole(0x2bbade1fL), tole(0x29fc6046L), tole(0x283e0a71L),
    tole(0x2d711cf4L), tole(0x2cb376c3L), tole(0x2ef5c89aL), tole(0x2f37a2adL),
    tole(0x709a8dc0L), tole(0x7158e7f7L), tole(0x731e59aeL), tole(0x72dc3399L),
    tole(0x7793251cL), tole(0x76514f2bL), tole(0x7417f172L), tole(0x75d59b45L),
    tole(0x7e89dc78L), tole(0x7f4bb64fL), tole(0x7d0d0816L), tole(0x7ccf6221L),
    tole(0x798074a4L), tole(0x78421e93L), tole(0x7a04a0caL), tole(0x7bc6cafdL),
    tole(0x6cbc2eb0L), tole(0x6d7e4487L), tole(0x6f38fadeL), tole(0x6efa90e9L),
    tole(0x6bb5866cL), tole(0x6a77ec5bL), tole(0x68315202L), tole(0x69f33835L),
    tole(0x62af7f08L), tole(0x636d153fL), tole(0x612bab66L), tole(0x60e9c151L),
    tole(0x65a6d7d4L), tole(0x6464bde3L), tole(0x662203baL), tole(0x67e0698dL),
    tole(0x48d7cb20L), tole(0x4915a117L), tole(0x4b531f4eL), tole(0x4a917579L),
    tole(0x4fde63fcL), tole(0x4e1c09cbL), tole(0x4c5ab792L), tole(0x4d98dda5L),
    tole(0x46c49a98L), tole(0x4706f0afL), tole(0x45404ef6L), tole(0x448224c1L),
    tole(0x41cd3244L), tole(0x400f5873L), tole(0x4249e62aL), tole(0x438b8c1dL),
    tole(0x54f16850L), tole(0x55330267L), tole(0x5775bc3eL), tole(0x56b7d609L),
    tole(0x53f8c08cL), tole(0x523aaabbL), tole(0x507c14e2L), tole(0x51be7ed5L),
    tole(0x5ae239e8L), tole(0x5b2053dfL), tole(0x5966ed86L), tole(0x58a487b1L),
    tole(0x5deb9134L), tole(0x5c29fb03L), tole(0x5e6f455aL), tole(0x5fad2f6dL),
    tole(0xe1351b80L), tole(0xe0f771b7L), tole(0xe2b1cfeeL), tole(0xe373a5d9L),
    tole(0xe63cb35cL), tole(0xe7fed96bL), tole(0xe5b86732L), tole(0xe47a0d05L),
    tole(0xef264a38L), tole(0xeee4200fL), tole(0xeca29e56L), tole(0xed60f461L),
    tole(0xe82fe2e4L), tole(0xe9ed88d3L), tole(0xebab368aL), tole(0xea695cbdL),
    tole(0xfd13b8f0L), tole(0xfcd1d2c7L), tole(0xfe976c9eL), tole(0xff5506a9L),
    tole(0xfa1a102cL), tole(0xfbd87a1bL), tole(0xf99ec442L), tole(0xf85cae75L),
    tole(0xf300e948L), tole(0xf2c2837fL), tole(0xf0843d26L), tole(0xf1465711L),
    tole(0xf4094194L), tole(0xf5cb2ba3L), tole(0xf78d95faL), tole(0xf64fffcdL),
    tole(0xd9785d60L), tole(0xd8ba3757L), tole(0xdafc890eL), tole(0xdb3ee339L),
    tole(0xde71f5bcL), tole(0xdfb39f8bL), tole(0xddf521d2L), tole(0xdc374be5L),
    tole(0xd76b0cd8L), tole(0xd6a966efL), tole(0xd4efd8b6L), tole(0xd52db281L),
    tole(0xd062a404L), tole(0xd1a0ce33L), tole(0xd3e6706aL), tole(0xd2241a5dL),
    tole(0xc55efe10L), tole(0xc49c9427L), tole(0xc6da2a7eL), tole(0xc7184049L),
    tole(0xc25756ccL), tole(0xc3953cfbL), tole(0xc1d382a2L), tole(0xc011e895L),
    tole(0xcb4dafa8L), tole(0xca8fc59fL), tole(0xc8c97bc6L), tole(0xc90b11f1L),
    tole(0xcc440774L), tole(0xcd866d43L), tole(0xcfc0d31aL), tole(0xce02b92dL),
    tole(0x91af9640L), tole(0x906dfc77L), tole(0x922b422eL), tole(0x93e92819L),
    tole(0x96a63e9cL), tole(0x976454abL), tole(0x9522eaf2L), tole(0x94e080c5L),
    tole(0x9fbcc7f8L), tole(0x9e7eadcfL), tole(0x9c381396L), tole(0x9dfa79a1L),
    tole(0x98b56f24L), tole(0x99770513L), tole(0x9b31bb4aL), tole(0x9af3d17dL),
    tole(0x8d893530L), tole(0x8c4b5f07L), tole(0x8e0de15eL), tole(0x8fcf8b69L),
    tole(0x8a809decL), tole(0x8b42f7dbL), tole(0x89044982L), tole(0x88c623b5L),
    tole(0x839a6488L), tole(0x82580ebfL), tole(0x801eb0e6L), tole(0x81dcdad1L),
    tole(0x8493cc54L), tole(0x8551a663L), tole(0x8717183aL), tole(0x86d5720dL),
    tole(0xa9e2d0a0L), tole(0xa820ba97L), tole(0xaa6604ceL), tole(0xaba46ef9L),
    tole(0xaeeb787cL), tole(0xaf29124bL), tole(0xad6fac12L), tole(0xacadc625L),
    tole(0xa7f18118L), tole(0xa633eb2fL), tole(0xa4755576L), tole(0xa5b73f41L),
    tole(0xa0f829c4L), tole(0xa13a43f3L), tole(0xa37cfdaaL), tole(0xa2be979dL),
    tole(0xb5c473d0L), tole(0xb40619e7L), tole(0xb640a7beL), tole(0xb782cd89L),
    tole(0xb2cddb0cL), tole(0xb30fb13bL), tole(0xb1490f62L), tole(0xb08b6555L),
    tole(0xbbd72268L), tole(0xba15485fL), tole(0xb853f606L), tole(0xb9919c31L),
    tole(0xbcde8ab4L), tole(0xbd1ce083L), tole(0xbf5a5edaL), tole(0xbe9834edL)},
    {
    tole(0x00000000L), tole(0xb8bc6765L), tole(0xaa09c88bL), tole(0x12b5afeeL),
    tole(0x8f629757L), tole(0x37def032L), tole(0x256b5fdcL), tole(0x9dd738b9L),
    tole(0xc5b428efL), tole(0x7d084f8aL), tole(0x6fbde064L), tole(0xd7018701L),
    tole(0x4ad6bfb8L), tole(0xf26ad8ddL), tole(0xe0df7733L), tole(0x58631056L),
    tole(0x5019579fL), tole(0xe8a530faL), tole(0xfa109f14L), tole(0x42acf871L),
    tole(0xdf7bc0c8L), tole(0x67c7a7adL), tole(0x75720843L), tole(0xcdce6f26L),
    tole(0x95ad7f70L), tole(0x2d111815L), tole(0x3fa4b7fbL), tole(0x8718d09eL),
    tole(0x1acfe827L), tole(0xa2738f42L), tole(0xb0c620acL), tole(0x087a47c9L),
    tole(0xa032af3eL), tole(0x188ec85bL), tole(0x0a3b67b5L), tole(0xb28700d0L),
    tole(0x2f503869L), tole(0x97ec5f0cL), tole(0x8559f0e2L), tole(0x3de59787L),
    tole(0x658687d1L), tole(0xdd3ae0b4L), tole(0xcf8f4f5aL), tole(0x7733283fL),
    tole(0xeae41086L), tole(0x525877e3L), tole(0x40edd80dL), tole(0xf851bf68L),
    tole(0xf02bf8a1L), tole(0x48979fc4L), tole(0x5a22302aL), tole(0xe29e574fL),
    tole(0x7f496ff6L), tole(0xc7f50893L), tole(0xd540a77dL), tole(0x6dfcc018L),
    tole(0x359fd04eL), tole(0x8d23b72bL), tole(0x9f9618c5L), tole(0x272a7fa0L),
    tole(0xbafd4719L), tole(0x0241207cL), tole(0x10f48f92L), tole(0xa848e8f7L),
    tole(0x9b14583dL), tole(0x23a83f58L), tole(0x311d90b6L), tole(0x89a1f7d3L),
    tole(0x1476cf6aL), tole(0xaccaa80fL), tole(0xbe7f07e1L), tole(0x06c36084L),
    tole(0x5ea070d2L), tole(0xe61c17b7L), tole(0xf4a9b859L), tole(0x4c15df3cL),
    tole(0xd1c2e785L), tole(0x697e80e0L), tole(0x7bcb2f0eL), tole(0xc377486bL),
    tole(0xcb0d0fa2L), tole(0x73b168c7L), tole(0x6104c729L), tole(0xd9b8a04cL),
    tole(0x446f98f5L), tole(0xfcd3ff90L), tole(0xee66507eL), tole(0x56da371bL),
    tole(0x0eb9274dL), tole(0xb6054028L), tole(0xa4b0efc6L), tole(0x1c0c88a3L),
    tole(0x81dbb01aL), tole(0x3967d77fL), tole(0x2bd27891L), tole(0x936e1ff4L),
    tole(0x3b26f703L), tole(0x839a9066L), tole(0x912f3f88L), tole(0x299358edL),
    tole(0xb4446054L), tole(0x0cf80731L), tole(0x1e4da8dfL), tole(0xa6f1cfbaL),
    tole(0xfe92dfecL), tole(0x462eb889L), tole(0x549b1767L), tole(0xec277002L),
    tole(0x71f048bbL), tole(0xc94c2fdeL), tole(0xdbf98030L), tole(0x6345e755L),
    tole(0x6b3fa09cL), tole(0xd383c7f9L), tole(0xc1366817L), tole(0x798a0f72L),
    tole(0xe45d37cbL), tole(0x5ce150aeL), tole(0x4e54ff40L), tole(0xf6e89825L),
    tole(0xae8b8873L), tole(0x1637ef16L), tole(0x048240f8L), tole(0xbc3e279dL),
    tole(0x21e91f24L), tole(0x99557841L), tole(0x8be0d7afL), tole(0x335cb0caL),
    tole(0xed59b63bL), tole(0x55e5d15eL), tole(0x47507eb0L), tole(0xffec19d5L),
    tole(0x623b216cL), tole(0xda874609L), tole(0xc832e9e7L), tole(0x708e8e82L),
    tole(0x28ed9ed4L), tole(0x9051f9b1L), tole(0x82e4565fL), tole(0x3a58313aL),
    tole(0xa78f0983L), tole(0x1f336ee6L), tole(0x0d86c108L), tole(0xb53aa66dL),
    tole(0xbd40e1a4L), tole(0x05fc86c1L), tole(0x1749292fL), tole(0xaff54e4aL),
    tole(0x322276f3L), tole(0x8a9e1196L), tole(0x982bbe78L), tole(0x2097d91dL),
    tole(0x78f4c94bL), tole(0xc048ae2eL), tole(0xd2fd01c0L), tole(0x6a4166a5L),
    tole(0xf7965e1cL), tole(0x4f2a3979L), tole(0x5d9f9697L), tole(0xe523f1f2L),
    tole(0x4d6b1905L), tole(0xf5d77e60L), tole(0xe762d18eL), tole(0x5fdeb6ebL),
    tole(0xc2098e52L), tole(0x7ab5e937L), tole(0x680046d9L), tole(0xd0bc21bcL),
    tole(0x88df31eaL), tole(0x3063568fL), tole(0x22d6f961L), tole(0x9a6a9e04L),
    tole(0x07bda6bdL), tole(0xbf01c1d8L), tole(0xadb46e36L), tole(0x15080953L),
    tole(0x1d724e9aL), tole(0xa5ce29ffL), tole(0xb77b8611L), tole(0x0fc7e174L),
    tole(0x9210d9cdL), tole(0x2aacbea8L), tole(0x38191146L), tole(0x80a57623L),
    tole(0xd8c66675L), tole(0x607a0110L), tole(0x72cfaefeL), tole(0xca73c99bL),
    tole(0x57a4f122L), tole(0xef189647L), tole(0xfdad39a9L), tole(0x45115eccL),
    tole(0x764dee06L), tole(0xcef18963L), tole(0xdc44268dL), tole(0x64f841e8L),
    tole(0xf92f7951L), tole(0x41931e34L), tole(0x5326b1daL), tole(0xeb9ad6bfL),
    tole(0xb3f9c6e9L), tole(0x0b45a18cL), tole(0x19f00e62L), tole(0xa14c6907L),
    tole(0x3c9b51beL), tole(0x842736dbL), tole(0x96929935L), tole(0x2e2efe50L),
    tole(0x2654b999L), tole(0x9ee8defcL), tole(0x8c5d7112L), tole(0x34e11677L),
    tole(0xa9362eceL), tole(0x118a49abL), tole(0x033fe645L), tole(0xbb838120L),
    tole(0xe3e09176L), tole(0x5b5cf613L), tole(0x49e959fdL), tole(0xf1553e98L),
    tole(0x6c820621L), tole(0xd43e6144L), tole(0xc68bceaaL), tole(0x7e37a9cfL),
    tole(0xd67f4138L), tole(0x6ec3265dL), tole(0x7c7689b3L), tole(0xc4caeed6L),
    tole(0x591dd66fL), tole(0xe1a1b10aL), tole(0xf3141ee4L), tole(0x4ba87981L),
    tole(0x13cb69d7L), tole(0xab770eb2L), tole(0xb9c2a15cL), tole(0x017ec639L),
    tole(0x9ca9fe80L), tole(0x241599e5L), tole(0x36a0360bL), tole(0x8e1c516eL),
    tole(0x866616a7L), tole(0x3eda71c2L), tole(0x2c6fde2cL), tole(0x94d3b949L),
    tole(0x090481f0L), tole(0xb1b8e695L), tole(0xa30d497bL), tole(0x1bb12e1eL),
    tole(0x43d23e48L), tole(0xfb6e592dL), tole(0xe9dbf6c3L), tole(0x516791a6L),
    tole(0xccb0a91fL), tole(0x740cce7aL), tole(0x66b96194L), tole(0xde0506f1L)},
    {
    tole(0x00000000L), tole(0x3d6029b0L), tole(0x7ac05360L), tole(0x47a07ad0L),
    tole(0xf580a6c0L), tole(0xc8e08f70L), tole(0x8f40f5a0L), tole(0xb220dc10L),
    tole(0x30704bc1L), tole(0x0d106271L), tole(0x4ab018a1L), tole(0x77d03111L),
    tole(0xc5f0ed01L), tole(0xf890c4b1L), tole(0xbf30be61L), tole(0x825097d1L),
    tole(0x60e09782L), tole(0x5d80be32L), tole(0x1a20c4e2L), tole(0x2740ed52L),
    tole(0x95603142L), tole(0xa80018f2L), tole(0xefa06222L), tole(0xd2c04b92L),
    tole(0x5090dc43L), tole(0x6df0f5f3L), tole(0x2a508f23L), tole(0x1730a693L),
    tole(0xa5107a83L), tole(0x98705333L), tole(0xdfd029e3L), tole(0xe2b00053L),
    tole(0xc1c12f04L), tole(0xfca106b4L), tole(0xbb017c64L), tole(0x866155d4L),
    tole(0x344189c4L), tole(0x0921a074L), tole(0x4e81daa4L), tole(0x73e1f314L),
    tole(0xf1b164c5L), tole(0xccd14d75L), tole(0x8b7137a5L), tole(0xb6111e15L),
    tole(0x0431c205L), tole(0x3951ebb5L), tole(0x7ef19165L), tole(0x4391b8d5L),
    tole(0xa121b886L), tole(0x9c419136L), tole(0xdbe1ebe6L), tole(0xe681c256L),
    tole(0x54a11e46L), tole(0x69c137f6L), tole(0x2e614d26L), tole(0x13016496L),
    tole(0x9151f347L), tole(0xac31daf7L), tole(0xeb91a027L), tole(0xd6f18997L),
    tole(0x64d15587L), tole(0x59b17c37L), tole(0x1e1106e7L), tole(0x23712f57L),
    tole(0x58f35849L), tole(0x659371f9L), tole(0x22330b29L), tole(0x1f532299L),
    tole(0xad73fe89L), tole(0x9013d739L), tole(0xd7b3ade9L), tole(0xead38459L),
    tole(0x68831388L), tole(0x55e33a38L), tole(0x124340e8L), tole(0x2f236958L),
    tole(0x9d03b548L), tole(0xa0639cf8L), tole(0xe7c3e628L), tole(0xdaa3cf98L),
    tole(0x3813cfcbL), tole(0x0573e67bL), tole(0x42d39cabL), tole(0x7fb3b51bL),
    tole(0xcd93690bL), tole(0xf0f340bbL), tole(0xb7533a6bL), tole(0x8a3313dbL),
    tole(0x0863840aL), tole(0x3503adbaL), tole(0x72a3d76aL), tole(0x4fc3fedaL),
    tole(0xfde322caL), tole(0xc0830b7aL), tole(0x872371aaL), tole(0xba43581aL),
    tole(0x9932774dL), tole(0xa4525efdL), tole(0xe3f2242dL), tole(0xde920d9dL),
    tole(0x6cb2d18dL), tole(0x51d2f83dL), tole(0x167282edL), tole(0x2b12ab5dL),
    tole(0xa9423c8cL), tole(0x9422153cL), tole(0xd3826fecL), tole(0xeee2465cL),
    tole(0x5cc29a4cL), tole(0x61a2b3fcL), tole(0x2602c92cL), tole(0x1b62e09cL),
    tole(0xf9d2e0cfL), tole(0xc4b2c97fL), tole(0x8312b3afL), tole(0xbe729a1fL),
    tole(0x0c52460fL), tole(0x31326fbfL), tole(0x7692156fL), tole(0x4bf23cdfL),
    tole(0xc9a2ab0eL), tole(0xf4c282beL), tole(0xb362f86eL), tole(0x8e02d1deL),
    tole(0x3c220dceL), tole(0x0142247eL), tole(0x46e25eaeL), tole(0x7b82771eL),
    tole(0xb1e6b092L), tole(0x8c869922L), tole(0xcb26e3f2L), tole(0xf646ca42L),
    tole(0x44661652L), tole(0x79063fe2L), tole(0x3ea64532L), tole(0x03c66c82L),
    tole(0x8196fb53L), tole(0xbcf6d2e3L), tole(0xfb56a833L), tole(0xc6368183L),
    tole(0x74165d93L), tole(0x49767423L), tole(0x0ed60ef3L), tole(0x33b62743L),
    tole(0xd1062710L), tole(0xec660ea0L), tole(0xabc67470L), tole(0x96a65dc0L),
    tole(0x248681d0L), tole(0x19e6a860L), tole(0x5e46d2b0L), tole(0x6326fb00L),
    tole(0xe1766cd1L), tole(0xdc164561L), tole(0x9bb63fb1L), tole(0xa6d61601L),
    tole(0x14f6ca11L), tole(0x2996e3a1L), tole(0x6e369971L), tole(0x5356b0c1L),
    tole(0x70279f96L), tole(0x4d47b626L), tole(0x0ae7ccf6L), tole(0x3787e546L),
    tole(0x85a73956L), tole(0xb8c710e6L), tole(0xff676a36L), tole(0xc2074386L),
    tole(0x4057d457L), tole(0x7d37fde7L), tole(0x3a978737L), tole(0x07f7ae87L),
    tole(0xb5d77297L), tole(0x88b75b27L), tole(0xcf1721f7L), tole(0xf2770847L),
    tole(0x10c70814L), tole(0x2da721a4L), tole(0x6a075b74L), tole(0x576772c4L),
    tole(0xe547aed4L), tole(0xd8278764L), tole(0x9f87fdb4L), tole(0xa2e7d404L),
    tole(0x20b743d5L), tole(0x1dd76a65L), tole(0x5a7710b5L), tole(0x67173905L),
    tole(0xd537e515L), tole(0xe857cca5L), tole(0xaff7b675L), tole(0x92979fc5L),
    tole(0xe915e8dbL), tole(0xd475c16bL), tole(0x93d5bbbbL), tole(0xaeb5920bL),
    tole(0x1c954e1bL), tole(0x21f567abL), tole(0x66551d7bL), tole(0x5b3534cbL),
    tole(0xd965a31aL), tole(0xe4058aaaL), tole(0xa3a5f07aL), tole(0x9ec5d9caL),
    tole(0x2ce505daL), tole(0x11852c6aL), tole(0x562556baL), tole(0x6b457f0aL),
    tole(0x89f57f59L), tole(0xb49556e9L), tole(0xf3352c39L), tole(0xce550589L),
    tole(0x7c75d999L), tole(0x4115f029L), tole(0x06b58af9L), tole(0x3bd5a349L),
    tole(0xb9853498L), tole(0x84e51d28L), tole(0xc34567f8L), tole(0xfe254e48L),
    tole(0x4c059258L), tole(0x7165bbe8L), tole(0x36c5c138L), tole(0x0ba5e888L),
    tole(0x28d4c7dfL), tole(0x15b4ee6fL), tole(0x521494bfL), tole(0x6f74bd0fL),
    tole(0xdd54611fL), tole(0xe03448afL), tole(0xa794327fL), tole(0x9af41bcfL),
    tole(0x18a48c1eL), tole(0x25c4a5aeL), tole(0x6264df7eL), tole(0x5f04f6ceL),
    tole(0xed242adeL), tole(0xd044036eL), tole(0x97e479beL), tole(0xaa84500eL),
    tole(0x4834505dL), tole(0x755479edL), tole(0x32f4033dL), tole(0x0f942a8dL),
    tole(0xbdb4f69dL), tole(0x80d4df2dL), tole(0xc774a5fdL), tole(0xfa148c4dL),
    tole(0x78441b9cL), tole(0x4524322cL), tole(0x028448fcL), tole(0x3fe4614cL),
    tole(0x8dc4bd5cL), tole(0xb0a494ecL), tole(0xf704ee3cL), tole(0xca64c78cL)},
    {
    tole(0x00000000L), tole(0xcb5cd3a5L), tole(0x4dc8a10bL), tole(0x869472aeL),
    tole(0x9b914216L), tole(0x50cd91b3L), tole(0xd659e31dL), tole(0x1d0530b8L),
    tole(0xec53826dL), tole(0x270f51c8L), tole(0xa19b2366L), tole(0x6ac7f0c3L),
    tole(0x77c2c07bL), tole(0xbc9e13deL), tole(0x3a0a6170L), tole(0xf156b2d5L),
    tole(0x03d6029bL), tole(0xc88ad13eL), tole(0x4e1ea390L), tole(0x85427035L),
    tole(0x9847408dL), tole(0x531b9328L), tole(0xd58fe186L), tole(0x1ed33223L),
    tole(0xef8580f6L), tole(0x24d95353L), tole(0xa24d21fdL), tole(0x6911f258L),
    tole(0x7414c2e0L), tole(0xbf481145L), tole(0x39dc63ebL), tole(0xf280b04eL),
    tole(0x07ac0536L), tole(0xccf0d693L), tole(0x4a64a43dL), tole(0x81387798L),
    tole(0x9c3d4720L), tole(0x57619485L), tole(0xd1f5e62bL), tole(0x1aa9358eL),
    tole(0xebff875bL), tole(0x20a354feL), tole(0xa6372650L), tole(0x6d6bf5f5L),
    tole(0x706ec54dL), tole(0xbb3216e8L), tole(0x3da66446L), tole(0xf6fab7e3L),
    tole(0x047a07adL), tole(0xcf26d408L), tole(0x49b2a6a6L), tole(0x82ee7503L),
    tole(0x9feb45bbL), tole(0x54b7961eL), tole(0xd223e4b0L), tole(0x197f3715L),
    tole(0xe82985c0L), tole(0x23755665L), tole(0xa5e124cbL), tole(0x6ebdf76eL),
    tole(0x73b8c7d6L), tole(0xb8e41473L), tole(0x3e7066ddL), tole(0xf52cb578L),
    tole(0x0f580a6cL), tole(0xc404d9c9L), tole(0x4290ab67L), tole(0x89cc78c2L),
    tole(0x94c9487aL), tole(0x5f959bdfL), tole(0xd901e971L), tole(0x125d3ad4L),
    tole(0xe30b8801L), tole(0x28575ba4L), tole(0xaec3290aL), tole(0x659ffaafL),
    tole(0x789aca17L), tole(0xb3c619b2L), tole(0x35526b1cL), tole(0xfe0eb8b9L),
    tole(0x0c8e08f7L), tole(0xc7d2db52L), tole(0x4146a9fcL), tole(0x8a1a7a59L),
    tole(0x971f4ae1L), tole(0x5c439944L), tole(0xdad7ebeaL), tole(0x118b384fL),
    tole(0xe0dd8a9aL), tole(0x2b81593fL), tole(0xad152b91L), tole(0x6649f834L),
    tole(0x7b4cc88cL), tole(0xb0101b29L), tole(0x36846987L), tole(0xfdd8ba22L),
    tole(0x08f40f5aL), tole(0xc3a8dcffL), tole(0x453cae51L), tole(0x8e607df4L),
    tole(0x93654d4cL), tole(0x58399ee9L), tole(0xdeadec47L), tole(0x15f13fe2L),
    tole(0xe4a78d37L), tole(0x2ffb5e92L), tole(0xa96f2c3cL), tole(0x6233ff99L),
    tole(0x7f36cf21L), tole(0xb46a1c84L), tole(0x32fe6e2aL), tole(0xf9a2bd8fL),
    tole(0x0b220dc1L), tole(0xc07ede64L), tole(0x46eaaccaL), tole(0x8db67f6fL),
    tole(0x90b34fd7L), tole(0x5bef9c72L), tole(0xdd7beedcL), tole(0x16273d79L),
    tole(0xe7718facL), tole(0x2c2d5c09L), tole(0xaab92ea7L), tole(0x61e5fd02L),
    tole(0x7ce0cdbaL), tole(0xb7bc1e1fL), tole(0x31286cb1L), tole(0xfa74bf14L),
    tole(0x1eb014d8L), tole(0xd5ecc77dL), tole(0x5378b5d3L), tole(0x98246676L),
    tole(0x852156ceL), tole(0x4e7d856bL), tole(0xc8e9f7c5L), tole(0x03b52460L),
    tole(0xf2e396b5L), tole(0x39bf4510L), tole(0xbf2b37beL), tole(0x7477e41bL),
    tole(0x6972d4a3L), tole(0xa22e0706L), tole(0x24ba75a8L), tole(0xefe6a60dL),
    tole(0x1d661643L), tole(0xd63ac5e6L), tole(0x50aeb748L), tole(0x9bf264edL),
    tole(0x86f75455L), tole(0x4dab87f0L), tole(0xcb3ff55eL), tole(0x006326fbL),
    tole(0xf135942eL), tole(0x3a69478bL), tole(0xbcfd3525L), tole(0x77a1e680L),
    tole(0x6aa4d638L), tole(0xa1f8059dL), tole(0x276c7733L), tole(0xec30a496L),
    tole(0x191c11eeL), tole(0xd240c24bL), tole(0x54d4b0e5L), tole(0x9f886340L),
    tole(0x828d53f8L), tole(0x49d1805dL), tole(0xcf45f2f3L), tole(0x04192156L),
    tole(0xf54f9383L), tole(0x3e134026L), tole(0xb8873288L), tole(0x73dbe12dL),
    tole(0x6eded195L), tole(0xa5820230L), tole(0x2316709eL), tole(0xe84aa33bL),
    tole(0x1aca1375L), tole(0xd196c0d0L), tole(0x5702b27eL), tole(0x9c5e61dbL),
    tole(0x815b5163L), tole(0x4a0782c6L), tole(0xcc93f068L), tole(0x07cf23cdL),
    tole(0xf6999118L), tole(0x3dc542bdL), tole(0xbb513013L), tole(0x700de3b6L),
    tole(0x6d08d30eL), tole(0xa65400abL), tole(0x20c07205L), tole(0xeb9ca1a0L),
    tole(0x11e81eb4L), tole(0xdab4cd11L), tole(0x5c20bfbfL), tole(0x977c6c1aL),
    tole(0x8a795ca2L), tole(0x41258f07L), tole(0xc7b1fda9L), tole(0x0ced2e0cL),
    tole(0xfdbb9cd9L), tole(0x36e74f7cL), tole(0xb0733dd2L), tole(0x7b2fee77L),
    tole(0x662adecfL), tole(0xad760d6aL), tole(0x2be27fc4L), tole(0xe0beac61L),
    tole(0x123e1c2fL), tole(0xd962cf8aL), tole(0x5ff6bd24L), tole(0x94aa6e81L),
    tole(0x89af5e39L), tole(0x42f38d9cL), tole(0xc467ff32L), tole(0x0f3b2c97L),
    tole(0xfe6d9e42L), tole(0x35314de7L), tole(0xb3a53f49L), tole(0x78f9ececL),
    tole(0x65fcdc54L), tole(0xaea00ff1L), tole(0x28347d5fL), tole(0xe368aefaL),
    tole(0x16441b82L), tole(0xdd18c827L), tole(0x5b8cba89L), tole(0x90d0692cL),
    tole(0x8dd55994L), tole(0x46898a31L), tole(0xc01df89fL), tole(0x0b412b3aL),
    tole(0xfa1799efL), tole(0x314b4a4aL), tole(0xb7df38e4L), tole(0x7c83eb41L),
    tole(0x6186dbf9L), tole(0xaada085cL), tole(0x2c4e7af2L), tole(0xe712a957L),
    tole(0x15921919L), tole(0xdececabcL), tole(0x585ab812L), tole(0x93066bb7L),
    tole(0x8e035b0fL), tole(0x455f88aaL), tole(0xc3cbfa04L), tole(0x089729a1L),
    tole(0xf9c19b74L), tole(0x329d48d1L), tole(0xb4093a7fL), tole(0x7f55e9daL),
    tole(0x6250d962L), tole(0xa90c0ac7L), tole(0x2f987869L), tole(0xe4c4abccL)},
    {
    tole(0x00000000L), tole(0xa6770bb4L), tole(0x979f1129L), tole(0x31e81a9dL),
    tole(0xf44f2413L), tole(0x52382fa7L), tole(0x63d0353aL), tole(0xc5a73e8eL),
    tole(0x33ef4e67L), tole(0x959845d3L), tole(0xa4705f4eL), tole(0x020754faL),
    tole(0xc7a06a74L), tole(0x61d761c0L), tole(0x503f7b5dL), tole(0xf64870e9L),
    tole(0x67de9cceL), tole(0xc1a9977aL), tole(0xf0418de7L), tole(0x56368653L),
    tole(0x9391b8ddL), tole(0x35e6b369L), tole(0x040ea9f4L), tole(0xa279a240L),
    tole(0x5431d2a9L), tole(0xf246d91dL), tole(0xc3aec380L), tole(0x65d9c834L),
    tole(0xa07ef6baL), tole(0x0609fd0eL), tole(0x37e1e793L), tole(0x9196ec27L),
    tole(0xcfbd399cL), tole(0x69ca3228L), tole(0x582228b5L), tole(0xfe552301L),
    tole(0x3bf21d8fL), tole(0x9d85163bL), tole(0xac6d0ca6L), tole(0x0a1a0712L),
    tole(0xfc5277fbL), tole(0x5a257c4fL), tole(0x6bcd66d2L), tole(0xcdba6d66L),
    tole(0x081d53e8L), tole(0xae6a585cL), tole(0x9f8242c1L), tole(0x39f54975L),
    tole(0xa863a552L), tole(0x0e14aee6L), tole(0x3ffcb47bL), tole(0x998bbfcfL),
    tole(0x5c2c8141L), tole(0xfa5b8af5L), tole(0xcbb39068L), tole(0x6dc49bdcL),
    tole(0x9b8ceb35L), tole(0x3dfbe081L), tole(0x0c13fa1cL), tole(0xaa64f1a8L),
    tole(0x6fc3cf26L), tole(0xc9b4c492L), tole(0xf85cde0fL), tole(0x5e2bd5bbL),
    tole(0x440b7579L), tole(0xe27c7ecdL), tole(0xd3946450L), tole(0x75e36fe4L),
    tole(0xb044516aL), tole(0x16335adeL), tole(0x27db4043L), tole(0x81ac4bf7L),
    tole(0x77e43b1eL), tole(0xd19330aaL), tole(0xe07b2a37L), tole(0x460c2183L),
    tole(0x83ab1f0dL), tole(0x25dc14b9L), tole(0x14340e24L), tole(0xb2430590L),
    tole(0x23d5e9b7L), tole(0x85a2e203L), tole(0xb44af89eL), tole(0x123df32aL),
    tole(0xd79acda4L), tole(0x71edc610L), tole(0x4005dc8dL), tole(0xe672d739L),
    tole(0x103aa7d0L), tole(0xb64dac64L), tole(0x87a5b6f9L), tole(0x21d2bd4dL),
    tole(0xe47583c3L), tole(0x42028877L), tole(0x73ea92eaL), tole(0xd59d995eL),
    tole(0x8bb64ce5L), tole(0x2dc14751L), tole(0x1c295dccL), tole(0xba5e5678L),
    tole(0x7ff968f6L), tole(0xd98e6342L), tole(0xe86679dfL), tole(0x4e11726bL),
    tole(0xb8590282L), tole(0x1e2e0936L), tole(0x2fc613abL), tole(0x89b1181fL),
    tole(0x4c162691L), tole(0xea612d25L), tole(0xdb8937b8L), tole(0x7dfe3c0cL),
    tole(0xec68d02bL), tole(0x4a1fdb9fL), tole(0x7bf7c102L), tole(0xdd80cab6L),
    tole(0x1827f438L), tole(0xbe50ff8cL), tole(0x8fb8e511L), tole(0x29cfeea5L),
    tole(0xdf879e4cL), tole(0x79f095f8L), tole(0x48188f65L), tole(0xee6f84d1L),
    tole(0x2bc8ba5fL), tole(0x8dbfb1ebL), tole(0xbc57ab76L), tole(0x1a20a0c2L),
    tole(0x8816eaf2L), tole(0x2e61e146L), tole(0x1f89fbdbL), tole(0xb9fef06fL),
    tole(0x7c59cee1L), tole(0xda2ec555L), tole(0xebc6dfc8L), tole(0x4db1d47cL),
    tole(0xbbf9a495L), tole(0x1d8eaf21L), tole(0x2c66b5bcL), tole(0x8a11be08L),
    tole(0x4fb68086L), tole(0xe9c18b32L), tole(0xd82991afL), tole(0x7e5e9a1bL),
    tole(0xefc8763cL), tole(0x49bf7d88L), tole(0x78576715L), tole(0xde206ca1L),
    tole(0x1b87522fL), tole(0xbdf0599bL), tole(0x8c184306L), tole(0x2a6f48b2L),
    tole(0xdc27385bL), tole(0x7a5033efL), tole(0x4bb82972L), tole(0xedcf22c6L),
    tole(0x28681c48L), tole(0x8e1f17fcL), tole(0xbff70d61L), tole(0x198006d5L),
    tole(0x47abd36eL), tole(0xe1dcd8daL), tole(0xd034c247L), tole(0x7643c9f3L),
    tole(0xb3e4f77dL), tole(0x1593fcc9L), tole(0x247be654L), tole(0x820cede0L),
    tole(0x74449d09L), tole(0xd23396bdL), tole(0xe3db8c20L), tole(0x45ac8794L),
    tole(0x800bb91aL), tole(0x267cb2aeL), tole(0x1794a833L), tole(0xb1e3a387L),
    tole(0x20754fa0L), tole(0x86024414L), tole(0xb7ea5e89L), tole(0x119d553dL),
    tole(0xd43a6bb3L), tole(0x724d6007L), tole(0x43a57a9aL), tole(0xe5d2712eL),
    tole(0x139a01c7L), tole(0xb5ed0a73L), tole(0x840510eeL), tole(0x22721b5aL),
    tole(0xe7d525d4L), tole(0x41a22e60L), tole(0x704a34fdL), tole(0xd63d3f49L),
    tole(0xcc1d9f8bL), tole(0x6a6a943fL), tole(0x5b828ea2L), tole(0xfdf58516L),
    tole(0x3852bb98L), tole(0x9e25b02cL), tole(0xafcdaab1L), tole(0x09baa105L),
    tole(0xfff2d1ecL), tole(0x5985da58L), tole(0x686dc0c5L), tole(0xce1acb71L),
    tole(0x0bbdf5ffL), tole(0xadcafe4bL), tole(0x9c22e4d6L), tole(0x3a55ef62L),
    tole(0xabc30345L), tole(0x0db408f1L), tole(0x3c5c126cL), tole(0x9a2b19d8L),
    tole(0x5f8c2756L), tole(0xf9fb2ce2L), tole(0xc813367fL), tole(0x6e643dcbL),
    tole(0x982c4d22L), tole(0x3e5b4696L), tole(0x0fb35c0bL), tole(0xa9c457bfL),
    tole(0x6c636931L), tole(0xca146285L), tole(0xfbfc7818L), tole(0x5d8b73acL),
    tole(0x03a0a617L), tole(0xa5d7ada3L), tole(0x943fb73eL), tole(0x3248bc8aL),
    tole(0xf7ef8204L), tole(0x519889b0L), tole(0x6070932dL), tole(0xc6079899L),
    tole(0x304fe870L), tole(0x9638e3c4L), tole(0xa7d0f959L), tole(0x01a7f2edL),
    tole(0xc400cc63L), tole(0x6277c7d7L), tole(0x539fdd4aL), tole(0xf5e8d6feL),
    tole(0x647e3ad9L), tole(0xc209316dL), tole(0xf3e12bf0L), tole(0x55962044L),
    tole(0x90311ecaL), tole(0x3646157eL), tole(0x07ae0fe3L), tole(0xa1d90457L),
    tole(0x579174beL), tole(0xf1e67f0aL), tole(0xc00e6597L), tole(0x66796e23L),
    tole(0xa3de50adL), tole(0x05a95b19L), tole(0x34414184L), tole(0x92364a30L)},
    {
    tole(0x00000000L), tole(0xccaa009eL), tole(0x4225077dL), tole(0x8e8f07e3L),
    tole(0x844a0efaL), tole(0x48e00e64L), tole(0xc66f0987L), tole(0x0ac50919L),
    tole(0xd3e51bb5L), tole(0x1f4f1b2bL), tole(0x91c01cc8L), tole(0x5d6a1c56L),
    tole(0x57af154fL), tole(0x9b0515d1L), tole(0x158a1232L), tole(0xd92012acL),
    tole(0x7cbb312bL), tole(0xb01131b5L), tole(0x3e9e3656L), tole(0xf23436c8L),
    tole(0xf8f13fd1L), tole(0x345b3f4fL), tole(0xbad438acL), tole(0x767e3832L),
    tole(0xaf5e2a9eL), tole(0x63f42a00L), tole(0xed7b2de3L), tole(0x21d12d7dL),
    tole(0x2b142464L), tole(0xe7be24faL), tole(0x69312319L), tole(0xa59b2387L),
    tole(0xf9766256L), tole(0x35dc62c8L), tole(0xbb53652bL), tole(0x77f965b5L),
    tole(0x7d3c6cacL), tole(0xb1966c32L), tole(0x3f196bd1L), tole(0xf3b36b4fL),
    tole(0x2a9379e3L), tole(0xe639797dL), tole(0x68b67e9eL), tole(0xa41c7e00L),
    tole(0xaed97719L), tole(0x62737787L), tole(0xecfc7064L), tole(0x205670faL),
    tole(0x85cd537dL), tole(0x496753e3L), tole(0xc7e85400L), tole(0x0b42549eL),
    tole(0x01875d87L), tole(0xcd2d5d19L), tole(0x43a25afaL), tole(0x8f085a64L),
    tole(0x562848c8L), tole(0x9a824856L), tole(0x140d4fb5L), tole(0xd8a74f2bL),
    tole(0xd2624632L), tole(0x1ec846acL), tole(0x9047414fL), tole(0x5ced41d1L),
    tole(0x299dc2edL), tole(0xe537c273L), tole(0x6bb8c590L), tole(0xa712c50eL),
    tole(0xadd7cc17L), tole(0x617dcc89L), tole(0xeff2cb6aL), tole(0x2358cbf4L),
    tole(0xfa78d958L), tole(0x36d2d9c6L), tole(0xb85dde25L), tole(0x74f7debbL),
    tole(0x7e32d7a2L), tole(0xb298d73cL), tole(0x3c17d0dfL), tole(0xf0bdd041L),
    tole(0x5526f3c6L), tole(0x998cf358L), tole(0x1703f4bbL), tole(0xdba9f425L),
    tole(0xd16cfd3cL), tole(0x1dc6fda2L), tole(0x9349fa41L), tole(0x5fe3fadfL),
    tole(0x86c3e873L), tole(0x4a69e8edL), tole(0xc4e6ef0eL), tole(0x084cef90L),
    tole(0x0289e689L), tole(0xce23e617L), tole(0x40ace1f4L), tole(0x8c06e16aL),
    tole(0xd0eba0bbL), tole(0x1c41a025L), tole(0x92cea7c6L), tole(0x5e64a758L),
    tole(0x54a1ae41L), tole(0x980baedfL), tole(0x1684a93cL), tole(0xda2ea9a2L),
    tole(0x030ebb0eL), tole(0xcfa4bb90L), tole(0x412bbc73L), tole(0x8d81bcedL),
    tole(0x8744b5f4L), tole(0x4beeb56aL), tole(0xc561b289L), tole(0x09cbb217L),
    tole(0xac509190L), tole(0x60fa910eL), tole(0xee7596edL), tole(0x22df9673L),
    tole(0x281a9f6aL), tole(0xe4b09ff4L), tole(0x6a3f9817L), tole(0xa6959889L),
    tole(0x7fb58a25L), tole(0xb31f8abbL), tole(0x3d908d58L), tole(0xf13a8dc6L),
    tole(0xfbff84dfL), tole(0x37558441L), tole(0xb9da83a2L), tole(0x7570833cL),
    tole(0x533b85daL), tole(0x9f918544L), tole(0x111e82a7L), tole(0xddb48239L),
    tole(0xd7718b20L), tole(0x1bdb8bbeL), tole(0x95548c5dL), tole(0x59fe8cc3L),
    tole(0x80de9e6fL), tole(0x4c749ef1L), tole(0xc2fb9912L), tole(0x0e51998cL),
    tole(0x04949095L), tole(0xc83e900bL), tole(0x46b197e8L), tole(0x8a1b9776L),
    tole(0x2f80b4f1L), tole(0xe32ab46fL), tole(0x6da5b38cL), tole(0xa10fb312L),
    tole(0xabcaba0bL), tole(0x6760ba95L), tole(0xe9efbd76L), tole(0x2545bde8L),
    tole(0xfc65af44L), tole(0x30cfafdaL), tole(0xbe40a839L), tole(0x72eaa8a7L),
    tole(0x782fa1beL), tole(0xb485a120L), tole(0x3a0aa6c3L), tole(0xf6a0a65dL),
    tole(0xaa4de78cL), tole(0x66e7e712L), tole(0xe868e0f1L), tole(0x24c2e06fL),
    tole(0x2e07e976L), tole(0xe2ade9e8L), tole(0x6c22ee0bL), tole(0xa088ee95L),
    tole(0x79a8fc39L), tole(0xb502fca7L), tole(0x3b8dfb44L), tole(0xf727fbdaL),
    tole(0xfde2f2c3L), tole(0x3148f25dL), tole(0xbfc7f5beL), tole(0x736df520L),
    tole(0xd6f6d6a7L), tole(0x1a5cd639L), tole(0x94d3d1daL), tole(0x5879d144L),
    tole(0x52bcd85dL), tole(0x9e16d8c3L), tole(0x1099df20L), tole(0xdc33dfbeL),
    tole(0x0513cd12L), tole(0xc9b9cd8cL), tole(0x4736ca6fL), tole(0x8b9ccaf1L),
    tole(0x8159c3e8L), tole(0x4df3c376L), tole(0xc37cc495L), tole(0x0fd6c40bL),
    tole(0x7aa64737L), tole(0xb60c47a9L), tole(0x3883404aL), tole(0xf42940d4L),
    tole(0xfeec49cdL), tole(0x32464953L), tole(0xbcc94eb0L), tole(0x70634e2eL),
    tole(0xa9435c82L), tole(0x65e95c1cL), tole(0xeb665bffL), tole(0x27cc5b61L),
    tole(0x2d095278L), tole(0xe1a352e6L), tole(0x6f2c5505L), tole(0xa386559bL),
    tole(0x061d761cL), tole(0xcab77682L), tole(0x44387161L), tole(0x889271ffL),
    tole(0x825778e6L), tole(0x4efd7878L), tole(0xc0727f9bL), tole(0x0cd87f05L),
    tole(0xd5f86da9L), tole(0x19526d37L), tole(0x97dd6ad4L), tole(0x5b776a4aL),
    tole(0x51b26353L), tole(0x9d1863cdL), tole(0x1397642eL), tole(0xdf3d64b0L),
    tole(0x83d02561L), tole(0x4f7a25ffL), tole(0xc1f5221cL), tole(0x0d5f2282L),
    tole(0x079a2b9bL), tole(0xcb302b05L), tole(0x45bf2ce6L), tole(0x89152c78L),
    tole(0x50353ed4L), tole(0x9c9f3e4aL), tole(0x121039a9L), tole(0xdeba3937L),
    tole(0xd47f302eL), tole(0x18d530b0L), tole(0x965a3753L), tole(0x5af037cdL),
    tole(0xff6b144aL), tole(0x33c114d4L), tole(0xbd4e1337L), tole(0x71e413a9L),
    tole(0x7b211ab0L), tole(0xb78b1a2eL), tole(0x39041dcdL), tole(0xf5ae1d53L),
    tole(0x2c8e0fffL), tole(0xe0240f61L), tole(0x6eab0882L), tole(0xa201081cL),
    tole(0xa8c40105L), tole(0x646e019bL), tole(0xeae10678L), tole(0x264b06e6L)},
    };
    static u32   crc32table_be[8][256] = {{
    tobe(0x00000000L), tobe(0x04c11db7L), tobe(0x09823b6eL), tobe(0x0d4326d9L),
    tobe(0x130476dcL), tobe(0x17c56b6bL), tobe(0x1a864db2L), tobe(0x1e475005L),
    tobe(0x2608edb8L), tobe(0x22c9f00fL), tobe(0x2f8ad6d6L), tobe(0x2b4bcb61L),
    tobe(0x350c9b64L), tobe(0x31cd86d3L), tobe(0x3c8ea00aL), tobe(0x384fbdbdL),
    tobe(0x4c11db70L), tobe(0x48d0c6c7L), tobe(0x4593e01eL), tobe(0x4152fda9L),
    tobe(0x5f15adacL), tobe(0x5bd4b01bL), tobe(0x569796c2L), tobe(0x52568b75L),
    tobe(0x6a1936c8L), tobe(0x6ed82b7fL), tobe(0x639b0da6L), tobe(0x675a1011L),
    tobe(0x791d4014L), tobe(0x7ddc5da3L), tobe(0x709f7b7aL), tobe(0x745e66cdL),
    tobe(0x9823b6e0L), tobe(0x9ce2ab57L), tobe(0x91a18d8eL), tobe(0x95609039L),
    tobe(0x8b27c03cL), tobe(0x8fe6dd8bL), tobe(0x82a5fb52L), tobe(0x8664e6e5L),
    tobe(0xbe2b5b58L), tobe(0xbaea46efL), tobe(0xb7a96036L), tobe(0xb3687d81L),
    tobe(0xad2f2d84L), tobe(0xa9ee3033L), tobe(0xa4ad16eaL), tobe(0xa06c0b5dL),
    tobe(0xd4326d90L), tobe(0xd0f37027L), tobe(0xddb056feL), tobe(0xd9714b49L),
    tobe(0xc7361b4cL), tobe(0xc3f706fbL), tobe(0xceb42022L), tobe(0xca753d95L),
    tobe(0xf23a8028L), tobe(0xf6fb9d9fL), tobe(0xfbb8bb46L), tobe(0xff79a6f1L),
    tobe(0xe13ef6f4L), tobe(0xe5ffeb43L), tobe(0xe8bccd9aL), tobe(0xec7dd02dL),
    tobe(0x34867077L), tobe(0x30476dc0L), tobe(0x3d044b19L), tobe(0x39c556aeL),
    tobe(0x278206abL), tobe(0x23431b1cL), tobe(0x2e003dc5L), tobe(0x2ac12072L),
    tobe(0x128e9dcfL), tobe(0x164f8078L), tobe(0x1b0ca6a1L), tobe(0x1fcdbb16L),
    tobe(0x018aeb13L), tobe(0x054bf6a4L), tobe(0x0808d07dL), tobe(0x0cc9cdcaL),
    tobe(0x7897ab07L), tobe(0x7c56b6b0L), tobe(0x71159069L), tobe(0x75d48ddeL),
    tobe(0x6b93dddbL), tobe(0x6f52c06cL), tobe(0x6211e6b5L), tobe(0x66d0fb02L),
    tobe(0x5e9f46bfL), tobe(0x5a5e5b08L), tobe(0x571d7dd1L), tobe(0x53dc6066L),
    tobe(0x4d9b3063L), tobe(0x495a2dd4L), tobe(0x44190b0dL), tobe(0x40d816baL),
    tobe(0xaca5c697L), tobe(0xa864db20L), tobe(0xa527fdf9L), tobe(0xa1e6e04eL),
    tobe(0xbfa1b04bL), tobe(0xbb60adfcL), tobe(0xb6238b25L), tobe(0xb2e29692L),
    tobe(0x8aad2b2fL), tobe(0x8e6c3698L), tobe(0x832f1041L), tobe(0x87ee0df6L),
    tobe(0x99a95df3L), tobe(0x9d684044L), tobe(0x902b669dL), tobe(0x94ea7b2aL),
    tobe(0xe0b41de7L), tobe(0xe4750050L), tobe(0xe9362689L), tobe(0xedf73b3eL),
    tobe(0xf3b06b3bL), tobe(0xf771768cL), tobe(0xfa325055L), tobe(0xfef34de2L),
    tobe(0xc6bcf05fL), tobe(0xc27dede8L), tobe(0xcf3ecb31L), tobe(0xcbffd686L),
    tobe(0xd5b88683L), tobe(0xd1799b34L), tobe(0xdc3abdedL), tobe(0xd8fba05aL),
    tobe(0x690ce0eeL), tobe(0x6dcdfd59L), tobe(0x608edb80L), tobe(0x644fc637L),
    tobe(0x7a089632L), tobe(0x7ec98b85L), tobe(0x738aad5cL), tobe(0x774bb0ebL),
    tobe(0x4f040d56L), tobe(0x4bc510e1L), tobe(0x46863638L), tobe(0x42472b8fL),
    tobe(0x5c007b8aL), tobe(0x58c1663dL), tobe(0x558240e4L), tobe(0x51435d53L),
    tobe(0x251d3b9eL), tobe(0x21dc2629L), tobe(0x2c9f00f0L), tobe(0x285e1d47L),
    tobe(0x36194d42L), tobe(0x32d850f5L), tobe(0x3f9b762cL), tobe(0x3b5a6b9bL),
    tobe(0x0315d626L), tobe(0x07d4cb91L), tobe(0x0a97ed48L), tobe(0x0e56f0ffL),
    tobe(0x1011a0faL), tobe(0x14d0bd4dL), tobe(0x19939b94L), tobe(0x1d528623L),
    tobe(0xf12f560eL), tobe(0xf5ee4bb9L), tobe(0xf8ad6d60L), tobe(0xfc6c70d7L),
    tobe(0xe22b20d2L), tobe(0xe6ea3d65L), tobe(0xeba91bbcL), tobe(0xef68060bL),
    tobe(0xd727bbb6L), tobe(0xd3e6a601L), tobe(0xdea580d8L), tobe(0xda649d6fL),
    tobe(0xc423cd6aL), tobe(0xc0e2d0ddL), tobe(0xcda1f604L), tobe(0xc960ebb3L),
    tobe(0xbd3e8d7eL), tobe(0xb9ff90c9L), tobe(0xb4bcb610L), tobe(0xb07daba7L),
    tobe(0xae3afba2L), tobe(0xaafbe615L), tobe(0xa7b8c0ccL), tobe(0xa379dd7bL),
    tobe(0x9b3660c6L), tobe(0x9ff77d71L), tobe(0x92b45ba8L), tobe(0x9675461fL),
    tobe(0x8832161aL), tobe(0x8cf30badL), tobe(0x81b02d74L), tobe(0x857130c3L),
    tobe(0x5d8a9099L), tobe(0x594b8d2eL), tobe(0x5408abf7L), tobe(0x50c9b640L),
    tobe(0x4e8ee645L), tobe(0x4a4ffbf2L), tobe(0x470cdd2bL), tobe(0x43cdc09cL),
    tobe(0x7b827d21L), tobe(0x7f436096L), tobe(0x7200464fL), tobe(0x76c15bf8L),
    tobe(0x68860bfdL), tobe(0x6c47164aL), tobe(0x61043093L), tobe(0x65c52d24L),
    tobe(0x119b4be9L), tobe(0x155a565eL), tobe(0x18197087L), tobe(0x1cd86d30L),
    tobe(0x029f3d35L), tobe(0x065e2082L), tobe(0x0b1d065bL), tobe(0x0fdc1becL),
    tobe(0x3793a651L), tobe(0x3352bbe6L), tobe(0x3e119d3fL), tobe(0x3ad08088L),
    tobe(0x2497d08dL), tobe(0x2056cd3aL), tobe(0x2d15ebe3L), tobe(0x29d4f654L),
    tobe(0xc5a92679L), tobe(0xc1683bceL), tobe(0xcc2b1d17L), tobe(0xc8ea00a0L),
    tobe(0xd6ad50a5L), tobe(0xd26c4d12L), tobe(0xdf2f6bcbL), tobe(0xdbee767cL),
    tobe(0xe3a1cbc1L), tobe(0xe760d676L), tobe(0xea23f0afL), tobe(0xeee2ed18L),
    tobe(0xf0a5bd1dL), tobe(0xf464a0aaL), tobe(0xf9278673L), tobe(0xfde69bc4L),
    tobe(0x89b8fd09L), tobe(0x8d79e0beL), tobe(0x803ac667L), tobe(0x84fbdbd0L),
    tobe(0x9abc8bd5L), tobe(0x9e7d9662L), tobe(0x933eb0bbL), tobe(0x97ffad0cL),
    tobe(0xafb010b1L), tobe(0xab710d06L), tobe(0xa6322bdfL), tobe(0xa2f33668L),
    tobe(0xbcb4666dL), tobe(0xb8757bdaL), tobe(0xb5365d03L), tobe(0xb1f740b4L)},
    {
    tobe(0x00000000L), tobe(0xd219c1dcL), tobe(0xa0f29e0fL), tobe(0x72eb5fd3L),
    tobe(0x452421a9L), tobe(0x973de075L), tobe(0xe5d6bfa6L), tobe(0x37cf7e7aL),
    tobe(0x8a484352L), tobe(0x5851828eL), tobe(0x2abadd5dL), tobe(0xf8a31c81L),
    tobe(0xcf6c62fbL), tobe(0x1d75a327L), tobe(0x6f9efcf4L), tobe(0xbd873d28L),
    tobe(0x10519b13L), tobe(0xc2485acfL), tobe(0xb0a3051cL), tobe(0x62bac4c0L),
    tobe(0x5575babaL), tobe(0x876c7b66L), tobe(0xf58724b5L), tobe(0x279ee569L),
    tobe(0x9a19d841L), tobe(0x4800199dL), tobe(0x3aeb464eL), tobe(0xe8f28792L),
    tobe(0xdf3df9e8L), tobe(0x0d243834L), tobe(0x7fcf67e7L), tobe(0xadd6a63bL),
    tobe(0x20a33626L), tobe(0xf2baf7faL), tobe(0x8051a829L), tobe(0x524869f5L),
    tobe(0x6587178fL), tobe(0xb79ed653L), tobe(0xc5758980L), tobe(0x176c485cL),
    tobe(0xaaeb7574L), tobe(0x78f2b4a8L), tobe(0x0a19eb7bL), tobe(0xd8002aa7L),
    tobe(0xefcf54ddL), tobe(0x3dd69501L), tobe(0x4f3dcad2L), tobe(0x9d240b0eL),
    tobe(0x30f2ad35L), tobe(0xe2eb6ce9L), tobe(0x9000333aL), tobe(0x4219f2e6L),
    tobe(0x75d68c9cL), tobe(0xa7cf4d40L), tobe(0xd5241293L), tobe(0x073dd34fL),
    tobe(0xbabaee67L), tobe(0x68a32fbbL), tobe(0x1a487068L), tobe(0xc851b1b4L),
    tobe(0xff9ecfceL), tobe(0x2d870e12L), tobe(0x5f6c51c1L), tobe(0x8d75901dL),
    tobe(0x41466c4cL), tobe(0x935fad90L), tobe(0xe1b4f243L), tobe(0x33ad339fL),
    tobe(0x04624de5L), tobe(0xd67b8c39L), tobe(0xa490d3eaL), tobe(0x76891236L),
    tobe(0xcb0e2f1eL), tobe(0x1917eec2L), tobe(0x6bfcb111L), tobe(0xb9e570cdL),
    tobe(0x8e2a0eb7L), tobe(0x5c33cf6bL), tobe(0x2ed890b8L), tobe(0xfcc15164L),
    tobe(0x5117f75fL), tobe(0x830e3683L), tobe(0xf1e56950L), tobe(0x23fca88cL),
    tobe(0x1433d6f6L), tobe(0xc62a172aL), tobe(0xb4c148f9L), tobe(0x66d88925L),
    tobe(0xdb5fb40dL), tobe(0x094675d1L), tobe(0x7bad2a02L), tobe(0xa9b4ebdeL),
    tobe(0x9e7b95a4L), tobe(0x4c625478L), tobe(0x3e890babL), tobe(0xec90ca77L),
    tobe(0x61e55a6aL), tobe(0xb3fc9bb6L), tobe(0xc117c465L), tobe(0x130e05b9L),
    tobe(0x24c17bc3L), tobe(0xf6d8ba1fL), tobe(0x8433e5ccL), tobe(0x562a2410L),
    tobe(0xebad1938L), tobe(0x39b4d8e4L), tobe(0x4b5f8737L), tobe(0x994646ebL),
    tobe(0xae893891L), tobe(0x7c90f94dL), tobe(0x0e7ba69eL), tobe(0xdc626742L),
    tobe(0x71b4c179L), tobe(0xa3ad00a5L), tobe(0xd1465f76L), tobe(0x035f9eaaL),
    tobe(0x3490e0d0L), tobe(0xe689210cL), tobe(0x94627edfL), tobe(0x467bbf03L),
    tobe(0xfbfc822bL), tobe(0x29e543f7L), tobe(0x5b0e1c24L), tobe(0x8917ddf8L),
    tobe(0xbed8a382L), tobe(0x6cc1625eL), tobe(0x1e2a3d8dL), tobe(0xcc33fc51L),
    tobe(0x828cd898L), tobe(0x50951944L), tobe(0x227e4697L), tobe(0xf067874bL),
    tobe(0xc7a8f931L), tobe(0x15b138edL), tobe(0x675a673eL), tobe(0xb543a6e2L),
    tobe(0x08c49bcaL), tobe(0xdadd5a16L), tobe(0xa83605c5L), tobe(0x7a2fc419L),
    tobe(0x4de0ba63L), tobe(0x9ff97bbfL), tobe(0xed12246cL), tobe(0x3f0be5b0L),
    tobe(0x92dd438bL), tobe(0x40c48257L), tobe(0x322fdd84L), tobe(0xe0361c58L),
    tobe(0xd7f96222L), tobe(0x05e0a3feL), tobe(0x770bfc2dL), tobe(0xa5123df1L),
    tobe(0x189500d9L), tobe(0xca8cc105L), tobe(0xb8679ed6L), tobe(0x6a7e5f0aL),
    tobe(0x5db12170L), tobe(0x8fa8e0acL), tobe(0xfd43bf7fL), tobe(0x2f5a7ea3L),
    tobe(0xa22feebeL), tobe(0x70362f62L), tobe(0x02dd70b1L), tobe(0xd0c4b16dL),
    tobe(0xe70bcf17L), tobe(0x35120ecbL), tobe(0x47f95118L), tobe(0x95e090c4L),
    tobe(0x2867adecL), tobe(0xfa7e6c30L), tobe(0x889533e3L), tobe(0x5a8cf23fL),
    tobe(0x6d438c45L), tobe(0xbf5a4d99L), tobe(0xcdb1124aL), tobe(0x1fa8d396L),
    tobe(0xb27e75adL), tobe(0x6067b471L), tobe(0x128ceba2L), tobe(0xc0952a7eL),
    tobe(0xf75a5404L), tobe(0x254395d8L), tobe(0x57a8ca0bL), tobe(0x85b10bd7L),
    tobe(0x383636ffL), tobe(0xea2ff723L), tobe(0x98c4a8f0L), tobe(0x4add692cL),
    tobe(0x7d121756L), tobe(0xaf0bd68aL), tobe(0xdde08959L), tobe(0x0ff94885L),
    tobe(0xc3cab4d4L), tobe(0x11d37508L), tobe(0x63382adbL), tobe(0xb121eb07L),
    tobe(0x86ee957dL), tobe(0x54f754a1L), tobe(0x261c0b72L), tobe(0xf405caaeL),
    tobe(0x4982f786L), tobe(0x9b9b365aL), tobe(0xe9706989L), tobe(0x3b69a855L),
    tobe(0x0ca6d62fL), tobe(0xdebf17f3L), tobe(0xac544820L), tobe(0x7e4d89fcL),
    tobe(0xd39b2fc7L), tobe(0x0182ee1bL), tobe(0x7369b1c8L), tobe(0xa1707014L),
    tobe(0x96bf0e6eL), tobe(0x44a6cfb2L), tobe(0x364d9061L), tobe(0xe45451bdL),
    tobe(0x59d36c95L), tobe(0x8bcaad49L), tobe(0xf921f29aL), tobe(0x2b383346L),
    tobe(0x1cf74d3cL), tobe(0xceee8ce0L), tobe(0xbc05d333L), tobe(0x6e1c12efL),
    tobe(0xe36982f2L), tobe(0x3170432eL), tobe(0x439b1cfdL), tobe(0x9182dd21L),
    tobe(0xa64da35bL), tobe(0x74546287L), tobe(0x06bf3d54L), tobe(0xd4a6fc88L),
    tobe(0x6921c1a0L), tobe(0xbb38007cL), tobe(0xc9d35fafL), tobe(0x1bca9e73L),
    tobe(0x2c05e009L), tobe(0xfe1c21d5L), tobe(0x8cf77e06L), tobe(0x5eeebfdaL),
    tobe(0xf33819e1L), tobe(0x2121d83dL), tobe(0x53ca87eeL), tobe(0x81d34632L),
    tobe(0xb61c3848L), tobe(0x6405f994L), tobe(0x16eea647L), tobe(0xc4f7679bL),
    tobe(0x79705ab3L), tobe(0xab699b6fL), tobe(0xd982c4bcL), tobe(0x0b9b0560L),
    tobe(0x3c547b1aL), tobe(0xee4dbac6L), tobe(0x9ca6e515L), tobe(0x4ebf24c9L)},
    {
    tobe(0x00000000L), tobe(0x01d8ac87L), tobe(0x03b1590eL), tobe(0x0269f589L),
    tobe(0x0762b21cL), tobe(0x06ba1e9bL), tobe(0x04d3eb12L), tobe(0x050b4795L),
    tobe(0x0ec56438L), tobe(0x0f1dc8bfL), tobe(0x0d743d36L), tobe(0x0cac91b1L),
    tobe(0x09a7d624L), tobe(0x087f7aa3L), tobe(0x0a168f2aL), tobe(0x0bce23adL),
    tobe(0x1d8ac870L), tobe(0x1c5264f7L), tobe(0x1e3b917eL), tobe(0x1fe33df9L),
    tobe(0x1ae87a6cL), tobe(0x1b30d6ebL), tobe(0x19592362L), tobe(0x18818fe5L),
    tobe(0x134fac48L), tobe(0x129700cfL), tobe(0x10fef546L), tobe(0x112659c1L),
    tobe(0x142d1e54L), tobe(0x15f5b2d3L), tobe(0x179c475aL), tobe(0x1644ebddL),
    tobe(0x3b1590e0L), tobe(0x3acd3c67L), tobe(0x38a4c9eeL), tobe(0x397c6569L),
    tobe(0x3c7722fcL), tobe(0x3daf8e7bL), tobe(0x3fc67bf2L), tobe(0x3e1ed775L),
    tobe(0x35d0f4d8L), tobe(0x3408585fL), tobe(0x3661add6L), tobe(0x37b90151L),
    tobe(0x32b246c4L), tobe(0x336aea43L), tobe(0x31031fcaL), tobe(0x30dbb34dL),
    tobe(0x269f5890L), tobe(0x2747f417L), tobe(0x252e019eL), tobe(0x24f6ad19L),
    tobe(0x21fdea8cL), tobe(0x2025460bL), tobe(0x224cb382L), tobe(0x23941f05L),
    tobe(0x285a3ca8L), tobe(0x2982902fL), tobe(0x2beb65a6L), tobe(0x2a33c921L),
    tobe(0x2f388eb4L), tobe(0x2ee02233L), tobe(0x2c89d7baL), tobe(0x2d517b3dL),
    tobe(0x762b21c0L), tobe(0x77f38d47L), tobe(0x759a78ceL), tobe(0x7442d449L),
    tobe(0x714993dcL), tobe(0x70913f5bL), tobe(0x72f8cad2L), tobe(0x73206655L),
    tobe(0x78ee45f8L), tobe(0x7936e97fL), tobe(0x7b5f1cf6L), tobe(0x7a87b071L),
    tobe(0x7f8cf7e4L), tobe(0x7e545b63L), tobe(0x7c3daeeaL), tobe(0x7de5026dL),
    tobe(0x6ba1e9b0L), tobe(0x6a794537L), tobe(0x6810b0beL), tobe(0x69c81c39L),
    tobe(0x6cc35bacL), tobe(0x6d1bf72bL), tobe(0x6f7202a2L), tobe(0x6eaaae25L),
    tobe(0x65648d88L), tobe(0x64bc210fL), tobe(0x66d5d486L), tobe(0x670d7801L),
    tobe(0x62063f94L), tobe(0x63de9313L), tobe(0x61b7669aL), tobe(0x606fca1dL),
    tobe(0x4d3eb120L), tobe(0x4ce61da7L), tobe(0x4e8fe82eL), tobe(0x4f5744a9L),
    tobe(0x4a5c033cL), tobe(0x4b84afbbL), tobe(0x49ed5a32L), tobe(0x4835f6b5L),
    tobe(0x43fbd518L), tobe(0x4223799fL), tobe(0x404a8c16L), tobe(0x41922091L),
    tobe(0x44996704L), tobe(0x4541cb83L), tobe(0x47283e0aL), tobe(0x46f0928dL),
    tobe(0x50b47950L), tobe(0x516cd5d7L), tobe(0x5305205eL), tobe(0x52dd8cd9L),
    tobe(0x57d6cb4cL), tobe(0x560e67cbL), tobe(0x54679242L), tobe(0x55bf3ec5L),
    tobe(0x5e711d68L), tobe(0x5fa9b1efL), tobe(0x5dc04466L), tobe(0x5c18e8e1L),
    tobe(0x5913af74L), tobe(0x58cb03f3L), tobe(0x5aa2f67aL), tobe(0x5b7a5afdL),
    tobe(0xec564380L), tobe(0xed8eef07L), tobe(0xefe71a8eL), tobe(0xee3fb609L),
    tobe(0xeb34f19cL), tobe(0xeaec5d1bL), tobe(0xe885a892L), tobe(0xe95d0415L),
    tobe(0xe29327b8L), tobe(0xe34b8b3fL), tobe(0xe1227eb6L), tobe(0xe0fad231L),
    tobe(0xe5f195a4L), tobe(0xe4293923L), tobe(0xe640ccaaL), tobe(0xe798602dL),
    tobe(0xf1dc8bf0L), tobe(0xf0042777L), tobe(0xf26dd2feL), tobe(0xf3b57e79L),
    tobe(0xf6be39ecL), tobe(0xf766956bL), tobe(0xf50f60e2L), tobe(0xf4d7cc65L),
    tobe(0xff19efc8L), tobe(0xfec1434fL), tobe(0xfca8b6c6L), tobe(0xfd701a41L),
    tobe(0xf87b5dd4L), tobe(0xf9a3f153L), tobe(0xfbca04daL), tobe(0xfa12a85dL),
    tobe(0xd743d360L), tobe(0xd69b7fe7L), tobe(0xd4f28a6eL), tobe(0xd52a26e9L),
    tobe(0xd021617cL), tobe(0xd1f9cdfbL), tobe(0xd3903872L), tobe(0xd24894f5L),
    tobe(0xd986b758L), tobe(0xd85e1bdfL), tobe(0xda37ee56L), tobe(0xdbef42d1L),
    tobe(0xdee40544L), tobe(0xdf3ca9c3L), tobe(0xdd555c4aL), tobe(0xdc8df0cdL),
    tobe(0xcac91b10L), tobe(0xcb11b797L), tobe(0xc978421eL), tobe(0xc8a0ee99L),
    tobe(0xcdaba90cL), tobe(0xcc73058bL), tobe(0xce1af002L), tobe(0xcfc25c85L),
    tobe(0xc40c7f28L), tobe(0xc5d4d3afL), tobe(0xc7bd2626L), tobe(0xc6658aa1L),
    tobe(0xc36ecd34L), tobe(0xc2b661b3L), tobe(0xc0df943aL), tobe(0xc10738bdL),
    tobe(0x9a7d6240L), tobe(0x9ba5cec7L), tobe(0x99cc3b4eL), tobe(0x981497c9L),
    tobe(0x9d1fd05cL), tobe(0x9cc77cdbL), tobe(0x9eae8952L), tobe(0x9f7625d5L),
    tobe(0x94b80678L), tobe(0x9560aaffL), tobe(0x97095f76L), tobe(0x96d1f3f1L),
    tobe(0x93dab464L), tobe(0x920218e3L), tobe(0x906bed6aL), tobe(0x91b341edL),
    tobe(0x87f7aa30L), tobe(0x862f06b7L), tobe(0x8446f33eL), tobe(0x859e5fb9L),
    tobe(0x8095182cL), tobe(0x814db4abL), tobe(0x83244122L), tobe(0x82fceda5L),
    tobe(0x8932ce08L), tobe(0x88ea628fL), tobe(0x8a839706L), tobe(0x8b5b3b81L),
    tobe(0x8e507c14L), tobe(0x8f88d093L), tobe(0x8de1251aL), tobe(0x8c39899dL),
    tobe(0xa168f2a0L), tobe(0xa0b05e27L), tobe(0xa2d9abaeL), tobe(0xa3010729L),
    tobe(0xa60a40bcL), tobe(0xa7d2ec3bL), tobe(0xa5bb19b2L), tobe(0xa463b535L),
    tobe(0xafad9698L), tobe(0xae753a1fL), tobe(0xac1ccf96L), tobe(0xadc46311L),
    tobe(0xa8cf2484L), tobe(0xa9178803L), tobe(0xab7e7d8aL), tobe(0xaaa6d10dL),
    tobe(0xbce23ad0L), tobe(0xbd3a9657L), tobe(0xbf5363deL), tobe(0xbe8bcf59L),
    tobe(0xbb8088ccL), tobe(0xba58244bL), tobe(0xb831d1c2L), tobe(0xb9e97d45L),
    tobe(0xb2275ee8L), tobe(0xb3fff26fL), tobe(0xb19607e6L), tobe(0xb04eab61L),
    tobe(0xb545ecf4L), tobe(0xb49d4073L), tobe(0xb6f4b5faL), tobe(0xb72c197dL)},
    {
    tobe(0x00000000L), tobe(0xdc6d9ab7L), tobe(0xbc1a28d9L), tobe(0x6077b26eL),
    tobe(0x7cf54c05L), tobe(0xa098d6b2L), tobe(0xc0ef64dcL), tobe(0x1c82fe6bL),
    tobe(0xf9ea980aL), tobe(0x258702bdL), tobe(0x45f0b0d3L), tobe(0x999d2a64L),
    tobe(0x851fd40fL), tobe(0x59724eb8L), tobe(0x3905fcd6L), tobe(0xe5686661L),
    tobe(0xf7142da3L), tobe(0x2b79b714L), tobe(0x4b0e057aL), tobe(0x97639fcdL),
    tobe(0x8be161a6L), tobe(0x578cfb11L), tobe(0x37fb497fL), tobe(0xeb96d3c8L),
    tobe(0x0efeb5a9L), tobe(0xd2932f1eL), tobe(0xb2e49d70L), tobe(0x6e8907c7L),
    tobe(0x720bf9acL), tobe(0xae66631bL), tobe(0xce11d175L), tobe(0x127c4bc2L),
    tobe(0xeae946f1L), tobe(0x3684dc46L), tobe(0x56f36e28L), tobe(0x8a9ef49fL),
    tobe(0x961c0af4L), tobe(0x4a719043L), tobe(0x2a06222dL), tobe(0xf66bb89aL),
    tobe(0x1303defbL), tobe(0xcf6e444cL), tobe(0xaf19f622L), tobe(0x73746c95L),
    tobe(0x6ff692feL), tobe(0xb39b0849L), tobe(0xd3ecba27L), tobe(0x0f812090L),
    tobe(0x1dfd6b52L), tobe(0xc190f1e5L), tobe(0xa1e7438bL), tobe(0x7d8ad93cL),
    tobe(0x61082757L), tobe(0xbd65bde0L), tobe(0xdd120f8eL), tobe(0x017f9539L),
    tobe(0xe417f358L), tobe(0x387a69efL), tobe(0x580ddb81L), tobe(0x84604136L),
    tobe(0x98e2bf5dL), tobe(0x448f25eaL), tobe(0x24f89784L), tobe(0xf8950d33L),
    tobe(0xd1139055L), tobe(0x0d7e0ae2L), tobe(0x6d09b88cL), tobe(0xb164223bL),
    tobe(0xade6dc50L), tobe(0x718b46e7L), tobe(0x11fcf489L), tobe(0xcd916e3eL),
    tobe(0x28f9085fL), tobe(0xf49492e8L), tobe(0x94e32086L), tobe(0x488eba31L),
    tobe(0x540c445aL), tobe(0x8861deedL), tobe(0xe8166c83L), tobe(0x347bf634L),
    tobe(0x2607bdf6L), tobe(0xfa6a2741L), tobe(0x9a1d952fL), tobe(0x46700f98L),
    tobe(0x5af2f1f3L), tobe(0x869f6b44L), tobe(0xe6e8d92aL), tobe(0x3a85439dL),
    tobe(0xdfed25fcL), tobe(0x0380bf4bL), tobe(0x63f70d25L), tobe(0xbf9a9792L),
    tobe(0xa31869f9L), tobe(0x7f75f34eL), tobe(0x1f024120L), tobe(0xc36fdb97L),
    tobe(0x3bfad6a4L), tobe(0xe7974c13L), tobe(0x87e0fe7dL), tobe(0x5b8d64caL),
    tobe(0x470f9aa1L), tobe(0x9b620016L), tobe(0xfb15b278L), tobe(0x277828cfL),
    tobe(0xc2104eaeL), tobe(0x1e7dd419L), tobe(0x7e0a6677L), tobe(0xa267fcc0L),
    tobe(0xbee502abL), tobe(0x6288981cL), tobe(0x02ff2a72L), tobe(0xde92b0c5L),
    tobe(0xcceefb07L), tobe(0x108361b0L), tobe(0x70f4d3deL), tobe(0xac994969L),
    tobe(0xb01bb702L), tobe(0x6c762db5L), tobe(0x0c019fdbL), tobe(0xd06c056cL),
    tobe(0x3504630dL), tobe(0xe969f9baL), tobe(0x891e4bd4L), tobe(0x5573d163L),
    tobe(0x49f12f08L), tobe(0x959cb5bfL), tobe(0xf5eb07d1L), tobe(0x29869d66L),
    tobe(0xa6e63d1dL), tobe(0x7a8ba7aaL), tobe(0x1afc15c4L), tobe(0xc6918f73L),
    tobe(0xda137118L), tobe(0x067eebafL), tobe(0x660959c1L), tobe(0xba64c376L),
    tobe(0x5f0ca517L), tobe(0x83613fa0L), tobe(0xe3168dceL), tobe(0x3f7b1779L),
    tobe(0x23f9e912L), tobe(0xff9473a5L), tobe(0x9fe3c1cbL), tobe(0x438e5b7cL),
    tobe(0x51f210beL), tobe(0x8d9f8a09L), tobe(0xede83867L), tobe(0x3185a2d0L),
    tobe(0x2d075cbbL), tobe(0xf16ac60cL), tobe(0x911d7462L), tobe(0x4d70eed5L),
    tobe(0xa81888b4L), tobe(0x74751203L), tobe(0x1402a06dL), tobe(0xc86f3adaL),
    tobe(0xd4edc4b1L), tobe(0x08805e06L), tobe(0x68f7ec68L), tobe(0xb49a76dfL),
    tobe(0x4c0f7becL), tobe(0x9062e15bL), tobe(0xf0155335L), tobe(0x2c78c982L),
    tobe(0x30fa37e9L), tobe(0xec97ad5eL), tobe(0x8ce01f30L), tobe(0x508d8587L),
    tobe(0xb5e5e3e6L), tobe(0x69887951L), tobe(0x09ffcb3fL), tobe(0xd5925188L),
    tobe(0xc910afe3L), tobe(0x157d3554L), tobe(0x750a873aL), tobe(0xa9671d8dL),
    tobe(0xbb1b564fL), tobe(0x6776ccf8L), tobe(0x07017e96L), tobe(0xdb6ce421L),
    tobe(0xc7ee1a4aL), tobe(0x1b8380fdL), tobe(0x7bf43293L), tobe(0xa799a824L),
    tobe(0x42f1ce45L), tobe(0x9e9c54f2L), tobe(0xfeebe69cL), tobe(0x22867c2bL),
    tobe(0x3e048240L), tobe(0xe26918f7L), tobe(0x821eaa99L), tobe(0x5e73302eL),
    tobe(0x77f5ad48L), tobe(0xab9837ffL), tobe(0xcbef8591L), tobe(0x17821f26L),
    tobe(0x0b00e14dL), tobe(0xd76d7bfaL), tobe(0xb71ac994L), tobe(0x6b775323L),
    tobe(0x8e1f3542L), tobe(0x5272aff5L), tobe(0x32051d9bL), tobe(0xee68872cL),
    tobe(0xf2ea7947L), tobe(0x2e87e3f0L), tobe(0x4ef0519eL), tobe(0x929dcb29L),
    tobe(0x80e180ebL), tobe(0x5c8c1a5cL), tobe(0x3cfba832L), tobe(0xe0963285L),
    tobe(0xfc14cceeL), tobe(0x20795659L), tobe(0x400ee437L), tobe(0x9c637e80L),
    tobe(0x790b18e1L), tobe(0xa5668256L), tobe(0xc5113038L), tobe(0x197caa8fL),
    tobe(0x05fe54e4L), tobe(0xd993ce53L), tobe(0xb9e47c3dL), tobe(0x6589e68aL),
    tobe(0x9d1cebb9L), tobe(0x4171710eL), tobe(0x2106c360L), tobe(0xfd6b59d7L),
    tobe(0xe1e9a7bcL), tobe(0x3d843d0bL), tobe(0x5df38f65L), tobe(0x819e15d2L),
    tobe(0x64f673b3L), tobe(0xb89be904L), tobe(0xd8ec5b6aL), tobe(0x0481c1ddL),
    tobe(0x18033fb6L), tobe(0xc46ea501L), tobe(0xa419176fL), tobe(0x78748dd8L),
    tobe(0x6a08c61aL), tobe(0xb6655cadL), tobe(0xd612eec3L), tobe(0x0a7f7474L),
    tobe(0x16fd8a1fL), tobe(0xca9010a8L), tobe(0xaae7a2c6L), tobe(0x768a3871L),
    tobe(0x93e25e10L), tobe(0x4f8fc4a7L), tobe(0x2ff876c9L), tobe(0xf395ec7eL),
    tobe(0xef171215L), tobe(0x337a88a2L), tobe(0x530d3accL), tobe(0x8f60a07bL)},
    {
    tobe(0x00000000L), tobe(0x490d678dL), tobe(0x921acf1aL), tobe(0xdb17a897L),
    tobe(0x20f48383L), tobe(0x69f9e40eL), tobe(0xb2ee4c99L), tobe(0xfbe32b14L),
    tobe(0x41e90706L), tobe(0x08e4608bL), tobe(0xd3f3c81cL), tobe(0x9afeaf91L),
    tobe(0x611d8485L), tobe(0x2810e308L), tobe(0xf3074b9fL), tobe(0xba0a2c12L),
    tobe(0x83d20e0cL), tobe(0xcadf6981L), tobe(0x11c8c116L), tobe(0x58c5a69bL),
    tobe(0xa3268d8fL), tobe(0xea2bea02L), tobe(0x313c4295L), tobe(0x78312518L),
    tobe(0xc23b090aL), tobe(0x8b366e87L), tobe(0x5021c610L), tobe(0x192ca19dL),
    tobe(0xe2cf8a89L), tobe(0xabc2ed04L), tobe(0x70d54593L), tobe(0x39d8221eL),
    tobe(0x036501afL), tobe(0x4a686622L), tobe(0x917fceb5L), tobe(0xd872a938L),
    tobe(0x2391822cL), tobe(0x6a9ce5a1L), tobe(0xb18b4d36L), tobe(0xf8862abbL),
    tobe(0x428c06a9L), tobe(0x0b816124L), tobe(0xd096c9b3L), tobe(0x999bae3eL),
    tobe(0x6278852aL), tobe(0x2b75e2a7L), tobe(0xf0624a30L), tobe(0xb96f2dbdL),
    tobe(0x80b70fa3L), tobe(0xc9ba682eL), tobe(0x12adc0b9L), tobe(0x5ba0a734L),
    tobe(0xa0438c20L), tobe(0xe94eebadL), tobe(0x3259433aL), tobe(0x7b5424b7L),
    tobe(0xc15e08a5L), tobe(0x88536f28L), tobe(0x5344c7bfL), tobe(0x1a49a032L),
    tobe(0xe1aa8b26L), tobe(0xa8a7ecabL), tobe(0x73b0443cL), tobe(0x3abd23b1L),
    tobe(0x06ca035eL), tobe(0x4fc764d3L), tobe(0x94d0cc44L), tobe(0xddddabc9L),
    tobe(0x263e80ddL), tobe(0x6f33e750L), tobe(0xb4244fc7L), tobe(0xfd29284aL),
    tobe(0x47230458L), tobe(0x0e2e63d5L), tobe(0xd539cb42L), tobe(0x9c34accfL),
    tobe(0x67d787dbL), tobe(0x2edae056L), tobe(0xf5cd48c1L), tobe(0xbcc02f4cL),
    tobe(0x85180d52L), tobe(0xcc156adfL), tobe(0x1702c248L), tobe(0x5e0fa5c5L),
    tobe(0xa5ec8ed1L), tobe(0xece1e95cL), tobe(0x37f641cbL), tobe(0x7efb2646L),
    tobe(0xc4f10a54L), tobe(0x8dfc6dd9L), tobe(0x56ebc54eL), tobe(0x1fe6a2c3L),
    tobe(0xe40589d7L), tobe(0xad08ee5aL), tobe(0x761f46cdL), tobe(0x3f122140L),
    tobe(0x05af02f1L), tobe(0x4ca2657cL), tobe(0x97b5cdebL), tobe(0xdeb8aa66L),
    tobe(0x255b8172L), tobe(0x6c56e6ffL), tobe(0xb7414e68L), tobe(0xfe4c29e5L),
    tobe(0x444605f7L), tobe(0x0d4b627aL), tobe(0xd65ccaedL), tobe(0x9f51ad60L),
    tobe(0x64b28674L), tobe(0x2dbfe1f9L), tobe(0xf6a8496eL), tobe(0xbfa52ee3L),
    tobe(0x867d0cfdL), tobe(0xcf706b70L), tobe(0x1467c3e7L), tobe(0x5d6aa46aL),
    tobe(0xa6898f7eL), tobe(0xef84e8f3L), tobe(0x34934064L), tobe(0x7d9e27e9L),
    tobe(0xc7940bfbL), tobe(0x8e996c76L), tobe(0x558ec4e1L), tobe(0x1c83a36cL),
    tobe(0xe7608878L), tobe(0xae6deff5L), tobe(0x757a4762L), tobe(0x3c7720efL),
    tobe(0x0d9406bcL), tobe(0x44996131L), tobe(0x9f8ec9a6L), tobe(0xd683ae2bL),
    tobe(0x2d60853fL), tobe(0x646de2b2L), tobe(0xbf7a4a25L), tobe(0xf6772da8L),
    tobe(0x4c7d01baL), tobe(0x05706637L), tobe(0xde67cea0L), tobe(0x976aa92dL),
    tobe(0x6c898239L), tobe(0x2584e5b4L), tobe(0xfe934d23L), tobe(0xb79e2aaeL),
    tobe(0x8e4608b0L), tobe(0xc74b6f3dL), tobe(0x1c5cc7aaL), tobe(0x5551a027L),
    tobe(0xaeb28b33L), tobe(0xe7bfecbeL), tobe(0x3ca84429L), tobe(0x75a523a4L),
    tobe(0xcfaf0fb6L), tobe(0x86a2683bL), tobe(0x5db5c0acL), tobe(0x14b8a721L),
    tobe(0xef5b8c35L), tobe(0xa656ebb8L), tobe(0x7d41432fL), tobe(0x344c24a2L),
    tobe(0x0ef10713L), tobe(0x47fc609eL), tobe(0x9cebc809L), tobe(0xd5e6af84L),
    tobe(0x2e058490L), tobe(0x6708e31dL), tobe(0xbc1f4b8aL), tobe(0xf5122c07L),
    tobe(0x4f180015L), tobe(0x06156798L), tobe(0xdd02cf0fL), tobe(0x940fa882L),
    tobe(0x6fec8396L), tobe(0x26e1e41bL), tobe(0xfdf64c8cL), tobe(0xb4fb2b01L),
    tobe(0x8d23091fL), tobe(0xc42e6e92L), tobe(0x1f39c605L), tobe(0x5634a188L),
    tobe(0xadd78a9cL), tobe(0xe4daed11L), tobe(0x3fcd4586L), tobe(0x76c0220bL),
    tobe(0xccca0e19L), tobe(0x85c76994L), tobe(0x5ed0c103L), tobe(0x17dda68eL),
    tobe(0xec3e8d9aL), tobe(0xa533ea17L), tobe(0x7e244280L), tobe(0x3729250dL),
    tobe(0x0b5e05e2L), tobe(0x4253626fL), tobe(0x9944caf8L), tobe(0xd049ad75L),
    tobe(0x2baa8661L), tobe(0x62a7e1ecL), tobe(0xb9b0497bL), tobe(0xf0bd2ef6L),
    tobe(0x4ab702e4L), tobe(0x03ba6569L), tobe(0xd8adcdfeL), tobe(0x91a0aa73L),
    tobe(0x6a438167L), tobe(0x234ee6eaL), tobe(0xf8594e7dL), tobe(0xb15429f0L),
    tobe(0x888c0beeL), tobe(0xc1816c63L), tobe(0x1a96c4f4L), tobe(0x539ba379L),
    tobe(0xa878886dL), tobe(0xe175efe0L), tobe(0x3a624777L), tobe(0x736f20faL),
    tobe(0xc9650ce8L), tobe(0x80686b65L), tobe(0x5b7fc3f2L), tobe(0x1272a47fL),
    tobe(0xe9918f6bL), tobe(0xa09ce8e6L), tobe(0x7b8b4071L), tobe(0x328627fcL),
    tobe(0x083b044dL), tobe(0x413663c0L), tobe(0x9a21cb57L), tobe(0xd32cacdaL),
    tobe(0x28cf87ceL), tobe(0x61c2e043L), tobe(0xbad548d4L), tobe(0xf3d82f59L),
    tobe(0x49d2034bL), tobe(0x00df64c6L), tobe(0xdbc8cc51L), tobe(0x92c5abdcL),
    tobe(0x692680c8L), tobe(0x202be745L), tobe(0xfb3c4fd2L), tobe(0xb231285fL),
    tobe(0x8be90a41L), tobe(0xc2e46dccL), tobe(0x19f3c55bL), tobe(0x50fea2d6L),
    tobe(0xab1d89c2L), tobe(0xe210ee4fL), tobe(0x390746d8L), tobe(0x700a2155L),
    tobe(0xca000d47L), tobe(0x830d6acaL), tobe(0x581ac25dL), tobe(0x1117a5d0L),
    tobe(0xeaf48ec4L), tobe(0xa3f9e949L), tobe(0x78ee41deL), tobe(0x31e32653L)},
    {
    tobe(0x00000000L), tobe(0x1b280d78L), tobe(0x36501af0L), tobe(0x2d781788L),
    tobe(0x6ca035e0L), tobe(0x77883898L), tobe(0x5af02f10L), tobe(0x41d82268L),
    tobe(0xd9406bc0L), tobe(0xc26866b8L), tobe(0xef107130L), tobe(0xf4387c48L),
    tobe(0xb5e05e20L), tobe(0xaec85358L), tobe(0x83b044d0L), tobe(0x989849a8L),
    tobe(0xb641ca37L), tobe(0xad69c74fL), tobe(0x8011d0c7L), tobe(0x9b39ddbfL),
    tobe(0xdae1ffd7L), tobe(0xc1c9f2afL), tobe(0xecb1e527L), tobe(0xf799e85fL),
    tobe(0x6f01a1f7L), tobe(0x7429ac8fL), tobe(0x5951bb07L), tobe(0x4279b67fL),
    tobe(0x03a19417L), tobe(0x1889996fL), tobe(0x35f18ee7L), tobe(0x2ed9839fL),
    tobe(0x684289d9L), tobe(0x736a84a1L), tobe(0x5e129329L), tobe(0x453a9e51L),
    tobe(0x04e2bc39L), tobe(0x1fcab141L), tobe(0x32b2a6c9L), tobe(0x299aabb1L),
    tobe(0xb102e219L), tobe(0xaa2aef61L), tobe(0x8752f8e9L), tobe(0x9c7af591L),
    tobe(0xdda2d7f9L), tobe(0xc68ada81L), tobe(0xebf2cd09L), tobe(0xf0dac071L),
    tobe(0xde0343eeL), tobe(0xc52b4e96L), tobe(0xe853591eL), tobe(0xf37b5466L),
    tobe(0xb2a3760eL), tobe(0xa98b7b76L), tobe(0x84f36cfeL), tobe(0x9fdb6186L),
    tobe(0x0743282eL), tobe(0x1c6b2556L), tobe(0x311332deL), tobe(0x2a3b3fa6L),
    tobe(0x6be31dceL), tobe(0x70cb10b6L), tobe(0x5db3073eL), tobe(0x469b0a46L),
    tobe(0xd08513b2L), tobe(0xcbad1ecaL), tobe(0xe6d50942L), tobe(0xfdfd043aL),
    tobe(0xbc252652L), tobe(0xa70d2b2aL), tobe(0x8a753ca2L), tobe(0x915d31daL),
    tobe(0x09c57872L), tobe(0x12ed750aL), tobe(0x3f956282L), tobe(0x24bd6ffaL),
    tobe(0x65654d92L), tobe(0x7e4d40eaL), tobe(0x53355762L), tobe(0x481d5a1aL),
    tobe(0x66c4d985L), tobe(0x7decd4fdL), tobe(0x5094c375L), tobe(0x4bbcce0dL),
    tobe(0x0a64ec65L), tobe(0x114ce11dL), tobe(0x3c34f695L), tobe(0x271cfbedL),
    tobe(0xbf84b245L), tobe(0xa4acbf3dL), tobe(0x89d4a8b5L), tobe(0x92fca5cdL),
    tobe(0xd32487a5L), tobe(0xc80c8addL), tobe(0xe5749d55L), tobe(0xfe5c902dL),
    tobe(0xb8c79a6bL), tobe(0xa3ef9713L), tobe(0x8e97809bL), tobe(0x95bf8de3L),
    tobe(0xd467af8bL), tobe(0xcf4fa2f3L), tobe(0xe237b57bL), tobe(0xf91fb803L),
    tobe(0x6187f1abL), tobe(0x7aaffcd3L), tobe(0x57d7eb5bL), tobe(0x4cffe623L),
    tobe(0x0d27c44bL), tobe(0x160fc933L), tobe(0x3b77debbL), tobe(0x205fd3c3L),
    tobe(0x0e86505cL), tobe(0x15ae5d24L), tobe(0x38d64aacL), tobe(0x23fe47d4L),
    tobe(0x622665bcL), tobe(0x790e68c4L), tobe(0x54767f4cL), tobe(0x4f5e7234L),
    tobe(0xd7c63b9cL), tobe(0xccee36e4L), tobe(0xe196216cL), tobe(0xfabe2c14L),
    tobe(0xbb660e7cL), tobe(0xa04e0304L), tobe(0x8d36148cL), tobe(0x961e19f4L),
    tobe(0xa5cb3ad3L), tobe(0xbee337abL), tobe(0x939b2023L), tobe(0x88b32d5bL),
    tobe(0xc96b0f33L), tobe(0xd243024bL), tobe(0xff3b15c3L), tobe(0xe41318bbL),
    tobe(0x7c8b5113L), tobe(0x67a35c6bL), tobe(0x4adb4be3L), tobe(0x51f3469bL),
    tobe(0x102b64f3L), tobe(0x0b03698bL), tobe(0x267b7e03L), tobe(0x3d53737bL),
    tobe(0x138af0e4L), tobe(0x08a2fd9cL), tobe(0x25daea14L), tobe(0x3ef2e76cL),
    tobe(0x7f2ac504L), tobe(0x6402c87cL), tobe(0x497adff4L), tobe(0x5252d28cL),
    tobe(0xcaca9b24L), tobe(0xd1e2965cL), tobe(0xfc9a81d4L), tobe(0xe7b28cacL),
    tobe(0xa66aaec4L), tobe(0xbd42a3bcL), tobe(0x903ab434L), tobe(0x8b12b94cL),
    tobe(0xcd89b30aL), tobe(0xd6a1be72L), tobe(0xfbd9a9faL), tobe(0xe0f1a482L),
    tobe(0xa12986eaL), tobe(0xba018b92L), tobe(0x97799c1aL), tobe(0x8c519162L),
    tobe(0x14c9d8caL), tobe(0x0fe1d5b2L), tobe(0x2299c23aL), tobe(0x39b1cf42L),
    tobe(0x7869ed2aL), tobe(0x6341e052L), tobe(0x4e39f7daL), tobe(0x5511faa2L),
    tobe(0x7bc8793dL), tobe(0x60e07445L), tobe(0x4d9863cdL), tobe(0x56b06eb5L),
    tobe(0x17684cddL), tobe(0x0c4041a5L), tobe(0x2138562dL), tobe(0x3a105b55L),
    tobe(0xa28812fdL), tobe(0xb9a01f85L), tobe(0x94d8080dL), tobe(0x8ff00575L),
    tobe(0xce28271dL), tobe(0xd5002a65L), tobe(0xf8783dedL), tobe(0xe3503095L),
    tobe(0x754e2961L), tobe(0x6e662419L), tobe(0x431e3391L), tobe(0x58363ee9L),
    tobe(0x19ee1c81L), tobe(0x02c611f9L), tobe(0x2fbe0671L), tobe(0x34960b09L),
    tobe(0xac0e42a1L), tobe(0xb7264fd9L), tobe(0x9a5e5851L), tobe(0x81765529L),
    tobe(0xc0ae7741L), tobe(0xdb867a39L), tobe(0xf6fe6db1L), tobe(0xedd660c9L),
    tobe(0xc30fe356L), tobe(0xd827ee2eL), tobe(0xf55ff9a6L), tobe(0xee77f4deL),
    tobe(0xafafd6b6L), tobe(0xb487dbceL), tobe(0x99ffcc46L), tobe(0x82d7c13eL),
    tobe(0x1a4f8896L), tobe(0x016785eeL), tobe(0x2c1f9266L), tobe(0x37379f1eL),
    tobe(0x76efbd76L), tobe(0x6dc7b00eL), tobe(0x40bfa786L), tobe(0x5b97aafeL),
    tobe(0x1d0ca0b8L), tobe(0x0624adc0L), tobe(0x2b5cba48L), tobe(0x3074b730L),
    tobe(0x71ac9558L), tobe(0x6a849820L), tobe(0x47fc8fa8L), tobe(0x5cd482d0L),
    tobe(0xc44ccb78L), tobe(0xdf64c600L), tobe(0xf21cd188L), tobe(0xe934dcf0L),
    tobe(0xa8ecfe98L), tobe(0xb3c4f3e0L), tobe(0x9ebce468L), tobe(0x8594e910L),
    tobe(0xab4d6a8fL), tobe(0xb06567f7L), tobe(0x9d1d707fL), tobe(0x86357d07L),
    tobe(0xc7ed5f6fL), tobe(0xdcc55217L), tobe(0xf1bd459fL), tobe(0xea9548e7L),
    tobe(0x720d014fL), tobe(0x69250c37L), tobe(0x445d1bbfL), tobe(0x5f7516c7L),
    tobe(0x1ead34afL), tobe(0x058539d7L), tobe(0x28fd2e5fL), tobe(0x33d52327L)},
    {
    tobe(0x00000000L), tobe(0x4f576811L), tobe(0x9eaed022L), tobe(0xd1f9b833L),
    tobe(0x399cbdf3L), tobe(0x76cbd5e2L), tobe(0xa7326dd1L), tobe(0xe86505c0L),
    tobe(0x73397be6L), tobe(0x3c6e13f7L), tobe(0xed97abc4L), tobe(0xa2c0c3d5L),
    tobe(0x4aa5c615L), tobe(0x05f2ae04L), tobe(0xd40b1637L), tobe(0x9b5c7e26L),
    tobe(0xe672f7ccL), tobe(0xa9259fddL), tobe(0x78dc27eeL), tobe(0x378b4fffL),
    tobe(0xdfee4a3fL), tobe(0x90b9222eL), tobe(0x41409a1dL), tobe(0x0e17f20cL),
    tobe(0x954b8c2aL), tobe(0xda1ce43bL), tobe(0x0be55c08L), tobe(0x44b23419L),
    tobe(0xacd731d9L), tobe(0xe38059c8L), tobe(0x3279e1fbL), tobe(0x7d2e89eaL),
    tobe(0xc824f22fL), tobe(0x87739a3eL), tobe(0x568a220dL), tobe(0x19dd4a1cL),
    tobe(0xf1b84fdcL), tobe(0xbeef27cdL), tobe(0x6f169ffeL), tobe(0x2041f7efL),
    tobe(0xbb1d89c9L), tobe(0xf44ae1d8L), tobe(0x25b359ebL), tobe(0x6ae431faL),
    tobe(0x8281343aL), tobe(0xcdd65c2bL), tobe(0x1c2fe418L), tobe(0x53788c09L),
    tobe(0x2e5605e3L), tobe(0x61016df2L), tobe(0xb0f8d5c1L), tobe(0xffafbdd0L),
    tobe(0x17cab810L), tobe(0x589dd001L), tobe(0x89646832L), tobe(0xc6330023L),
    tobe(0x5d6f7e05L), tobe(0x12381614L), tobe(0xc3c1ae27L), tobe(0x8c96c636L),
    tobe(0x64f3c3f6L), tobe(0x2ba4abe7L), tobe(0xfa5d13d4L), tobe(0xb50a7bc5L),
    tobe(0x9488f9e9L), tobe(0xdbdf91f8L), tobe(0x0a2629cbL), tobe(0x457141daL),
    tobe(0xad14441aL), tobe(0xe2432c0bL), tobe(0x33ba9438L), tobe(0x7cedfc29L),
    tobe(0xe7b1820fL), tobe(0xa8e6ea1eL), tobe(0x791f522dL), tobe(0x36483a3cL),
    tobe(0xde2d3ffcL), tobe(0x917a57edL), tobe(0x4083efdeL), tobe(0x0fd487cfL),
    tobe(0x72fa0e25L), tobe(0x3dad6634L), tobe(0xec54de07L), tobe(0xa303b616L),
    tobe(0x4b66b3d6L), tobe(0x0431dbc7L), tobe(0xd5c863f4L), tobe(0x9a9f0be5L),
    tobe(0x01c375c3L), tobe(0x4e941dd2L), tobe(0x9f6da5e1L), tobe(0xd03acdf0L),
    tobe(0x385fc830L), tobe(0x7708a021L), tobe(0xa6f11812L), tobe(0xe9a67003L),
    tobe(0x5cac0bc6L), tobe(0x13fb63d7L), tobe(0xc202dbe4L), tobe(0x8d55b3f5L),
    tobe(0x6530b635L), tobe(0x2a67de24L), tobe(0xfb9e6617L), tobe(0xb4c90e06L),
    tobe(0x2f957020L), tobe(0x60c21831L), tobe(0xb13ba002L), tobe(0xfe6cc813L),
    tobe(0x1609cdd3L), tobe(0x595ea5c2L), tobe(0x88a71df1L), tobe(0xc7f075e0L),
    tobe(0xbadefc0aL), tobe(0xf589941bL), tobe(0x24702c28L), tobe(0x6b274439L),
    tobe(0x834241f9L), tobe(0xcc1529e8L), tobe(0x1dec91dbL), tobe(0x52bbf9caL),
    tobe(0xc9e787ecL), tobe(0x86b0effdL), tobe(0x574957ceL), tobe(0x181e3fdfL),
    tobe(0xf07b3a1fL), tobe(0xbf2c520eL), tobe(0x6ed5ea3dL), tobe(0x2182822cL),
    tobe(0x2dd0ee65L), tobe(0x62878674L), tobe(0xb37e3e47L), tobe(0xfc295656L),
    tobe(0x144c5396L), tobe(0x5b1b3b87L), tobe(0x8ae283b4L), tobe(0xc5b5eba5L),
    tobe(0x5ee99583L), tobe(0x11befd92L), tobe(0xc04745a1L), tobe(0x8f102db0L),
    tobe(0x67752870L), tobe(0x28224061L), tobe(0xf9dbf852L), tobe(0xb68c9043L),
    tobe(0xcba219a9L), tobe(0x84f571b8L), tobe(0x550cc98bL), tobe(0x1a5ba19aL),
    tobe(0xf23ea45aL), tobe(0xbd69cc4bL), tobe(0x6c907478L), tobe(0x23c71c69L),
    tobe(0xb89b624fL), tobe(0xf7cc0a5eL), tobe(0x2635b26dL), tobe(0x6962da7cL),
    tobe(0x8107dfbcL), tobe(0xce50b7adL), tobe(0x1fa90f9eL), tobe(0x50fe678fL),
    tobe(0xe5f41c4aL), tobe(0xaaa3745bL), tobe(0x7b5acc68L), tobe(0x340da479L),
    tobe(0xdc68a1b9L), tobe(0x933fc9a8L), tobe(0x42c6719bL), tobe(0x0d91198aL),
    tobe(0x96cd67acL), tobe(0xd99a0fbdL), tobe(0x0863b78eL), tobe(0x4734df9fL),
    tobe(0xaf51da5fL), tobe(0xe006b24eL), tobe(0x31ff0a7dL), tobe(0x7ea8626cL),
    tobe(0x0386eb86L), tobe(0x4cd18397L), tobe(0x9d283ba4L), tobe(0xd27f53b5L),
    tobe(0x3a1a5675L), tobe(0x754d3e64L), tobe(0xa4b48657L), tobe(0xebe3ee46L),
    tobe(0x70bf9060L), tobe(0x3fe8f871L), tobe(0xee114042L), tobe(0xa1462853L),
    tobe(0x49232d93L), tobe(0x06744582L), tobe(0xd78dfdb1L), tobe(0x98da95a0L),
    tobe(0xb958178cL), tobe(0xf60f7f9dL), tobe(0x27f6c7aeL), tobe(0x68a1afbfL),
    tobe(0x80c4aa7fL), tobe(0xcf93c26eL), tobe(0x1e6a7a5dL), tobe(0x513d124cL),
    tobe(0xca616c6aL), tobe(0x8536047bL), tobe(0x54cfbc48L), tobe(0x1b98d459L),
    tobe(0xf3fdd199L), tobe(0xbcaab988L), tobe(0x6d5301bbL), tobe(0x220469aaL),
    tobe(0x5f2ae040L), tobe(0x107d8851L), tobe(0xc1843062L), tobe(0x8ed35873L),
    tobe(0x66b65db3L), tobe(0x29e135a2L), tobe(0xf8188d91L), tobe(0xb74fe580L),
    tobe(0x2c139ba6L), tobe(0x6344f3b7L), tobe(0xb2bd4b84L), tobe(0xfdea2395L),
    tobe(0x158f2655L), tobe(0x5ad84e44L), tobe(0x8b21f677L), tobe(0xc4769e66L),
    tobe(0x717ce5a3L), tobe(0x3e2b8db2L), tobe(0xefd23581L), tobe(0xa0855d90L),
    tobe(0x48e05850L), tobe(0x07b73041L), tobe(0xd64e8872L), tobe(0x9919e063L),
    tobe(0x02459e45L), tobe(0x4d12f654L), tobe(0x9ceb4e67L), tobe(0xd3bc2676L),
    tobe(0x3bd923b6L), tobe(0x748e4ba7L), tobe(0xa577f394L), tobe(0xea209b85L),
    tobe(0x970e126fL), tobe(0xd8597a7eL), tobe(0x09a0c24dL), tobe(0x46f7aa5cL),
    tobe(0xae92af9cL), tobe(0xe1c5c78dL), tobe(0x303c7fbeL), tobe(0x7f6b17afL),
    tobe(0xe4376989L), tobe(0xab600198L), tobe(0x7a99b9abL), tobe(0x35ced1baL),
    tobe(0xddabd47aL), tobe(0x92fcbc6bL), tobe(0x43050458L), tobe(0x0c526c49L)},
    {
    tobe(0x00000000L), tobe(0x5ba1dccaL), tobe(0xb743b994L), tobe(0xece2655eL),
    tobe(0x6a466e9fL), tobe(0x31e7b255L), tobe(0xdd05d70bL), tobe(0x86a40bc1L),
    tobe(0xd48cdd3eL), tobe(0x8f2d01f4L), tobe(0x63cf64aaL), tobe(0x386eb860L),
    tobe(0xbecab3a1L), tobe(0xe56b6f6bL), tobe(0x09890a35L), tobe(0x5228d6ffL),
    tobe(0xadd8a7cbL), tobe(0xf6797b01L), tobe(0x1a9b1e5fL), tobe(0x413ac295L),
    tobe(0xc79ec954L), tobe(0x9c3f159eL), tobe(0x70dd70c0L), tobe(0x2b7cac0aL),
    tobe(0x79547af5L), tobe(0x22f5a63fL), tobe(0xce17c361L), tobe(0x95b61fabL),
    tobe(0x1312146aL), tobe(0x48b3c8a0L), tobe(0xa451adfeL), tobe(0xfff07134L),
    tobe(0x5f705221L), tobe(0x04d18eebL), tobe(0xe833ebb5L), tobe(0xb392377fL),
    tobe(0x35363cbeL), tobe(0x6e97e074L), tobe(0x8275852aL), tobe(0xd9d459e0L),
    tobe(0x8bfc8f1fL), tobe(0xd05d53d5L), tobe(0x3cbf368bL), tobe(0x671eea41L),
    tobe(0xe1bae180L), tobe(0xba1b3d4aL), tobe(0x56f95814L), tobe(0x0d5884deL),
    tobe(0xf2a8f5eaL), tobe(0xa9092920L), tobe(0x45eb4c7eL), tobe(0x1e4a90b4L),
    tobe(0x98ee9b75L), tobe(0xc34f47bfL), tobe(0x2fad22e1L), tobe(0x740cfe2bL),
    tobe(0x262428d4L), tobe(0x7d85f41eL), tobe(0x91679140L), tobe(0xcac64d8aL),
    tobe(0x4c62464bL), tobe(0x17c39a81L), tobe(0xfb21ffdfL), tobe(0xa0802315L),
    tobe(0xbee0a442L), tobe(0xe5417888L), tobe(0x09a31dd6L), tobe(0x5202c11cL),
    tobe(0xd4a6caddL), tobe(0x8f071617L), tobe(0x63e57349L), tobe(0x3844af83L),
    tobe(0x6a6c797cL), tobe(0x31cda5b6L), tobe(0xdd2fc0e8L), tobe(0x868e1c22L),
    tobe(0x002a17e3L), tobe(0x5b8bcb29L), tobe(0xb769ae77L), tobe(0xecc872bdL),
    tobe(0x13380389L), tobe(0x4899df43L), tobe(0xa47bba1dL), tobe(0xffda66d7L),
    tobe(0x797e6d16L), tobe(0x22dfb1dcL), tobe(0xce3dd482L), tobe(0x959c0848L),
    tobe(0xc7b4deb7L), tobe(0x9c15027dL), tobe(0x70f76723L), tobe(0x2b56bbe9L),
    tobe(0xadf2b028L), tobe(0xf6536ce2L), tobe(0x1ab109bcL), tobe(0x4110d576L),
    tobe(0xe190f663L), tobe(0xba312aa9L), tobe(0x56d34ff7L), tobe(0x0d72933dL),
    tobe(0x8bd698fcL), tobe(0xd0774436L), tobe(0x3c952168L), tobe(0x6734fda2L),
    tobe(0x351c2b5dL), tobe(0x6ebdf797L), tobe(0x825f92c9L), tobe(0xd9fe4e03L),
    tobe(0x5f5a45c2L), tobe(0x04fb9908L), tobe(0xe819fc56L), tobe(0xb3b8209cL),
    tobe(0x4c4851a8L), tobe(0x17e98d62L), tobe(0xfb0be83cL), tobe(0xa0aa34f6L),
    tobe(0x260e3f37L), tobe(0x7dafe3fdL), tobe(0x914d86a3L), tobe(0xcaec5a69L),
    tobe(0x98c48c96L), tobe(0xc365505cL), tobe(0x2f873502L), tobe(0x7426e9c8L),
    tobe(0xf282e209L), tobe(0xa9233ec3L), tobe(0x45c15b9dL), tobe(0x1e608757L),
    tobe(0x79005533L), tobe(0x22a189f9L), tobe(0xce43eca7L), tobe(0x95e2306dL),
    tobe(0x13463bacL), tobe(0x48e7e766L), tobe(0xa4058238L), tobe(0xffa45ef2L),
    tobe(0xad8c880dL), tobe(0xf62d54c7L), tobe(0x1acf3199L), tobe(0x416eed53L),
    tobe(0xc7cae692L), tobe(0x9c6b3a58L), tobe(0x70895f06L), tobe(0x2b2883ccL),
    tobe(0xd4d8f2f8L), tobe(0x8f792e32L), tobe(0x639b4b6cL), tobe(0x383a97a6L),
    tobe(0xbe9e9c67L), tobe(0xe53f40adL), tobe(0x09dd25f3L), tobe(0x527cf939L),
    tobe(0x00542fc6L), tobe(0x5bf5f30cL), tobe(0xb7179652L), tobe(0xecb64a98L),
    tobe(0x6a124159L), tobe(0x31b39d93L), tobe(0xdd51f8cdL), tobe(0x86f02407L),
    tobe(0x26700712L), tobe(0x7dd1dbd8L), tobe(0x9133be86L), tobe(0xca92624cL),
    tobe(0x4c36698dL), tobe(0x1797b547L), tobe(0xfb75d019L), tobe(0xa0d40cd3L),
    tobe(0xf2fcda2cL), tobe(0xa95d06e6L), tobe(0x45bf63b8L), tobe(0x1e1ebf72L),
    tobe(0x98bab4b3L), tobe(0xc31b6879L), tobe(0x2ff90d27L), tobe(0x7458d1edL),
    tobe(0x8ba8a0d9L), tobe(0xd0097c13L), tobe(0x3ceb194dL), tobe(0x674ac587L),
    tobe(0xe1eece46L), tobe(0xba4f128cL), tobe(0x56ad77d2L), tobe(0x0d0cab18L),
    tobe(0x5f247de7L), tobe(0x0485a12dL), tobe(0xe867c473L), tobe(0xb3c618b9L),
    tobe(0x35621378L), tobe(0x6ec3cfb2L), tobe(0x8221aaecL), tobe(0xd9807626L),
    tobe(0xc7e0f171L), tobe(0x9c412dbbL), tobe(0x70a348e5L), tobe(0x2b02942fL),
    tobe(0xada69feeL), tobe(0xf6074324L), tobe(0x1ae5267aL), tobe(0x4144fab0L),
    tobe(0x136c2c4fL), tobe(0x48cdf085L), tobe(0xa42f95dbL), tobe(0xff8e4911L),
    tobe(0x792a42d0L), tobe(0x228b9e1aL), tobe(0xce69fb44L), tobe(0x95c8278eL),
    tobe(0x6a3856baL), tobe(0x31998a70L), tobe(0xdd7bef2eL), tobe(0x86da33e4L),
    tobe(0x007e3825L), tobe(0x5bdfe4efL), tobe(0xb73d81b1L), tobe(0xec9c5d7bL),
    tobe(0xbeb48b84L), tobe(0xe515574eL), tobe(0x09f73210L), tobe(0x5256eedaL),
    tobe(0xd4f2e51bL), tobe(0x8f5339d1L), tobe(0x63b15c8fL), tobe(0x38108045L),
    tobe(0x9890a350L), tobe(0xc3317f9aL), tobe(0x2fd31ac4L), tobe(0x7472c60eL),
    tobe(0xf2d6cdcfL), tobe(0xa9771105L), tobe(0x4595745bL), tobe(0x1e34a891L),
    tobe(0x4c1c7e6eL), tobe(0x17bda2a4L), tobe(0xfb5fc7faL), tobe(0xa0fe1b30L),
    tobe(0x265a10f1L), tobe(0x7dfbcc3bL), tobe(0x9119a965L), tobe(0xcab875afL),
    tobe(0x3548049bL), tobe(0x6ee9d851L), tobe(0x820bbd0fL), tobe(0xd9aa61c5L),
    tobe(0x5f0e6a04L), tobe(0x04afb6ceL), tobe(0xe84dd390L), tobe(0xb3ec0f5aL),
    tobe(0xe1c4d9a5L), tobe(0xba65056fL), tobe(0x56876031L), tobe(0x0d26bcfbL),
    tobe(0x8b82b73aL), tobe(0xd0236bf0L), tobe(0x3cc10eaeL), tobe(0x6760d264L)},
    };
    static u32   crc32ctable_le[8][256] = {{
    tole(0x00000000L), tole(0xf26b8303L), tole(0xe13b70f7L), tole(0x1350f3f4L),
    tole(0xc79a971fL), tole(0x35f1141cL), tole(0x26a1e7e8L), tole(0xd4ca64ebL),
    tole(0x8ad958cfL), tole(0x78b2dbccL), tole(0x6be22838L), tole(0x9989ab3bL),
    tole(0x4d43cfd0L), tole(0xbf284cd3L), tole(0xac78bf27L), tole(0x5e133c24L),
    tole(0x105ec76fL), tole(0xe235446cL), tole(0xf165b798L), tole(0x030e349bL),
    tole(0xd7c45070L), tole(0x25afd373L), tole(0x36ff2087L), tole(0xc494a384L),
    tole(0x9a879fa0L), tole(0x68ec1ca3L), tole(0x7bbcef57L), tole(0x89d76c54L),
    tole(0x5d1d08bfL), tole(0xaf768bbcL), tole(0xbc267848L), tole(0x4e4dfb4bL),
    tole(0x20bd8edeL), tole(0xd2d60dddL), tole(0xc186fe29L), tole(0x33ed7d2aL),
    tole(0xe72719c1L), tole(0x154c9ac2L), tole(0x061c6936L), tole(0xf477ea35L),
    tole(0xaa64d611L), tole(0x580f5512L), tole(0x4b5fa6e6L), tole(0xb93425e5L),
    tole(0x6dfe410eL), tole(0x9f95c20dL), tole(0x8cc531f9L), tole(0x7eaeb2faL),
    tole(0x30e349b1L), tole(0xc288cab2L), tole(0xd1d83946L), tole(0x23b3ba45L),
    tole(0xf779deaeL), tole(0x05125dadL), tole(0x1642ae59L), tole(0xe4292d5aL),
    tole(0xba3a117eL), tole(0x4851927dL), tole(0x5b016189L), tole(0xa96ae28aL),
    tole(0x7da08661L), tole(0x8fcb0562L), tole(0x9c9bf696L), tole(0x6ef07595L),
    tole(0x417b1dbcL), tole(0xb3109ebfL), tole(0xa0406d4bL), tole(0x522bee48L),
    tole(0x86e18aa3L), tole(0x748a09a0L), tole(0x67dafa54L), tole(0x95b17957L),
    tole(0xcba24573L), tole(0x39c9c670L), tole(0x2a993584L), tole(0xd8f2b687L),
    tole(0x0c38d26cL), tole(0xfe53516fL), tole(0xed03a29bL), tole(0x1f682198L),
    tole(0x5125dad3L), tole(0xa34e59d0L), tole(0xb01eaa24L), tole(0x42752927L),
    tole(0x96bf4dccL), tole(0x64d4cecfL), tole(0x77843d3bL), tole(0x85efbe38L),
    tole(0xdbfc821cL), tole(0x2997011fL), tole(0x3ac7f2ebL), tole(0xc8ac71e8L),
    tole(0x1c661503L), tole(0xee0d9600L), tole(0xfd5d65f4L), tole(0x0f36e6f7L),
    tole(0x61c69362L), tole(0x93ad1061L), tole(0x80fde395L), tole(0x72966096L),
    tole(0xa65c047dL), tole(0x5437877eL), tole(0x4767748aL), tole(0xb50cf789L),
    tole(0xeb1fcbadL), tole(0x197448aeL), tole(0x0a24bb5aL), tole(0xf84f3859L),
    tole(0x2c855cb2L), tole(0xdeeedfb1L), tole(0xcdbe2c45L), tole(0x3fd5af46L),
    tole(0x7198540dL), tole(0x83f3d70eL), tole(0x90a324faL), tole(0x62c8a7f9L),
    tole(0xb602c312L), tole(0x44694011L), tole(0x5739b3e5L), tole(0xa55230e6L),
    tole(0xfb410cc2L), tole(0x092a8fc1L), tole(0x1a7a7c35L), tole(0xe811ff36L),
    tole(0x3cdb9bddL), tole(0xceb018deL), tole(0xdde0eb2aL), tole(0x2f8b6829L),
    tole(0x82f63b78L), tole(0x709db87bL), tole(0x63cd4b8fL), tole(0x91a6c88cL),
    tole(0x456cac67L), tole(0xb7072f64L), tole(0xa457dc90L), tole(0x563c5f93L),
    tole(0x082f63b7L), tole(0xfa44e0b4L), tole(0xe9141340L), tole(0x1b7f9043L),
    tole(0xcfb5f4a8L), tole(0x3dde77abL), tole(0x2e8e845fL), tole(0xdce5075cL),
    tole(0x92a8fc17L), tole(0x60c37f14L), tole(0x73938ce0L), tole(0x81f80fe3L),
    tole(0x55326b08L), tole(0xa759e80bL), tole(0xb4091bffL), tole(0x466298fcL),
    tole(0x1871a4d8L), tole(0xea1a27dbL), tole(0xf94ad42fL), tole(0x0b21572cL),
    tole(0xdfeb33c7L), tole(0x2d80b0c4L), tole(0x3ed04330L), tole(0xccbbc033L),
    tole(0xa24bb5a6L), tole(0x502036a5L), tole(0x4370c551L), tole(0xb11b4652L),
    tole(0x65d122b9L), tole(0x97baa1baL), tole(0x84ea524eL), tole(0x7681d14dL),
    tole(0x2892ed69L), tole(0xdaf96e6aL), tole(0xc9a99d9eL), tole(0x3bc21e9dL),
    tole(0xef087a76L), tole(0x1d63f975L), tole(0x0e330a81L), tole(0xfc588982L),
    tole(0xb21572c9L), tole(0x407ef1caL), tole(0x532e023eL), tole(0xa145813dL),
    tole(0x758fe5d6L), tole(0x87e466d5L), tole(0x94b49521L), tole(0x66df1622L),
    tole(0x38cc2a06L), tole(0xcaa7a905L), tole(0xd9f75af1L), tole(0x2b9cd9f2L),
    tole(0xff56bd19L), tole(0x0d3d3e1aL), tole(0x1e6dcdeeL), tole(0xec064eedL),
    tole(0xc38d26c4L), tole(0x31e6a5c7L), tole(0x22b65633L), tole(0xd0ddd530L),
    tole(0x0417b1dbL), tole(0xf67c32d8L), tole(0xe52cc12cL), tole(0x1747422fL),
    tole(0x49547e0bL), tole(0xbb3ffd08L), tole(0xa86f0efcL), tole(0x5a048dffL),
    tole(0x8ecee914L), tole(0x7ca56a17L), tole(0x6ff599e3L), tole(0x9d9e1ae0L),
    tole(0xd3d3e1abL), tole(0x21b862a8L), tole(0x32e8915cL), tole(0xc083125fL),
    tole(0x144976b4L), tole(0xe622f5b7L), tole(0xf5720643L), tole(0x07198540L),
    tole(0x590ab964L), tole(0xab613a67L), tole(0xb831c993L), tole(0x4a5a4a90L),
    tole(0x9e902e7bL), tole(0x6cfbad78L), tole(0x7fab5e8cL), tole(0x8dc0dd8fL),
    tole(0xe330a81aL), tole(0x115b2b19L), tole(0x020bd8edL), tole(0xf0605beeL),
    tole(0x24aa3f05L), tole(0xd6c1bc06L), tole(0xc5914ff2L), tole(0x37faccf1L),
    tole(0x69e9f0d5L), tole(0x9b8273d6L), tole(0x88d28022L), tole(0x7ab90321L),
    tole(0xae7367caL), tole(0x5c18e4c9L), tole(0x4f48173dL), tole(0xbd23943eL),
    tole(0xf36e6f75L), tole(0x0105ec76L), tole(0x12551f82L), tole(0xe03e9c81L),
    tole(0x34f4f86aL), tole(0xc69f7b69L), tole(0xd5cf889dL), tole(0x27a40b9eL),
    tole(0x79b737baL), tole(0x8bdcb4b9L), tole(0x988c474dL), tole(0x6ae7c44eL),
    tole(0xbe2da0a5L), tole(0x4c4623a6L), tole(0x5f16d052L), tole(0xad7d5351L)},
    {
    tole(0x00000000L), tole(0x13a29877L), tole(0x274530eeL), tole(0x34e7a899L),
    tole(0x4e8a61dcL), tole(0x5d28f9abL), tole(0x69cf5132L), tole(0x7a6dc945L),
    tole(0x9d14c3b8L), tole(0x8eb65bcfL), tole(0xba51f356L), tole(0xa9f36b21L),
    tole(0xd39ea264L), tole(0xc03c3a13L), tole(0xf4db928aL), tole(0xe7790afdL),
    tole(0x3fc5f181L), tole(0x2c6769f6L), tole(0x1880c16fL), tole(0x0b225918L),
    tole(0x714f905dL), tole(0x62ed082aL), tole(0x560aa0b3L), tole(0x45a838c4L),
    tole(0xa2d13239L), tole(0xb173aa4eL), tole(0x859402d7L), tole(0x96369aa0L),
    tole(0xec5b53e5L), tole(0xfff9cb92L), tole(0xcb1e630bL), tole(0xd8bcfb7cL),
    tole(0x7f8be302L), tole(0x6c297b75L), tole(0x58ced3ecL), tole(0x4b6c4b9bL),
    tole(0x310182deL), tole(0x22a31aa9L), tole(0x1644b230L), tole(0x05e62a47L),
    tole(0xe29f20baL), tole(0xf13db8cdL), tole(0xc5da1054L), tole(0xd6788823L),
    tole(0xac154166L), tole(0xbfb7d911L), tole(0x8b507188L), tole(0x98f2e9ffL),
    tole(0x404e1283L), tole(0x53ec8af4L), tole(0x670b226dL), tole(0x74a9ba1aL),
    tole(0x0ec4735fL), tole(0x1d66eb28L), tole(0x298143b1L), tole(0x3a23dbc6L),
    tole(0xdd5ad13bL), tole(0xcef8494cL), tole(0xfa1fe1d5L), tole(0xe9bd79a2L),
    tole(0x93d0b0e7L), tole(0x80722890L), tole(0xb4958009L), tole(0xa737187eL),
    tole(0xff17c604L), tole(0xecb55e73L), tole(0xd852f6eaL), tole(0xcbf06e9dL),
    tole(0xb19da7d8L), tole(0xa23f3fafL), tole(0x96d89736L), tole(0x857a0f41L),
    tole(0x620305bcL), tole(0x71a19dcbL), tole(0x45463552L), tole(0x56e4ad25L),
    tole(0x2c896460L), tole(0x3f2bfc17L), tole(0x0bcc548eL), tole(0x186eccf9L),
    tole(0xc0d23785L), tole(0xd370aff2L), tole(0xe797076bL), tole(0xf4359f1cL),
    tole(0x8e585659L), tole(0x9dface2eL), tole(0xa91d66b7L), tole(0xbabffec0L),
    tole(0x5dc6f43dL), tole(0x4e646c4aL), tole(0x7a83c4d3L), tole(0x69215ca4L),
    tole(0x134c95e1L), tole(0x00ee0d96L), tole(0x3409a50fL), tole(0x27ab3d78L),
    tole(0x809c2506L), tole(0x933ebd71L), tole(0xa7d915e8L), tole(0xb47b8d9fL),
    tole(0xce1644daL), tole(0xddb4dcadL), tole(0xe9537434L), tole(0xfaf1ec43L),
    tole(0x1d88e6beL), tole(0x0e2a7ec9L), tole(0x3acdd650L), tole(0x296f4e27L),
    tole(0x53028762L), tole(0x40a01f15L), tole(0x7447b78cL), tole(0x67e52ffbL),
    tole(0xbf59d487L), tole(0xacfb4cf0L), tole(0x981ce469L), tole(0x8bbe7c1eL),
    tole(0xf1d3b55bL), tole(0xe2712d2cL), tole(0xd69685b5L), tole(0xc5341dc2L),
    tole(0x224d173fL), tole(0x31ef8f48L), tole(0x050827d1L), tole(0x16aabfa6L),
    tole(0x6cc776e3L), tole(0x7f65ee94L), tole(0x4b82460dL), tole(0x5820de7aL),
    tole(0xfbc3faf9L), tole(0xe861628eL), tole(0xdc86ca17L), tole(0xcf245260L),
    tole(0xb5499b25L), tole(0xa6eb0352L), tole(0x920cabcbL), tole(0x81ae33bcL),
    tole(0x66d73941L), tole(0x7575a136L), tole(0x419209afL), tole(0x523091d8L),
    tole(0x285d589dL), tole(0x3bffc0eaL), tole(0x0f186873L), tole(0x1cbaf004L),
    tole(0xc4060b78L), tole(0xd7a4930fL), tole(0xe3433b96L), tole(0xf0e1a3e1L),
    tole(0x8a8c6aa4L), tole(0x992ef2d3L), tole(0xadc95a4aL), tole(0xbe6bc23dL),
    tole(0x5912c8c0L), tole(0x4ab050b7L), tole(0x7e57f82eL), tole(0x6df56059L),
    tole(0x1798a91cL), tole(0x043a316bL), tole(0x30dd99f2L), tole(0x237f0185L),
    tole(0x844819fbL), tole(0x97ea818cL), tole(0xa30d2915L), tole(0xb0afb162L),
    tole(0xcac27827L), tole(0xd960e050L), tole(0xed8748c9L), tole(0xfe25d0beL),
    tole(0x195cda43L), tole(0x0afe4234L), tole(0x3e19eaadL), tole(0x2dbb72daL),
    tole(0x57d6bb9fL), tole(0x447423e8L), tole(0x70938b71L), tole(0x63311306L),
    tole(0xbb8de87aL), tole(0xa82f700dL), tole(0x9cc8d894L), tole(0x8f6a40e3L),
    tole(0xf50789a6L), tole(0xe6a511d1L), tole(0xd242b948L), tole(0xc1e0213fL),
    tole(0x26992bc2L), tole(0x353bb3b5L), tole(0x01dc1b2cL), tole(0x127e835bL),
    tole(0x68134a1eL), tole(0x7bb1d269L), tole(0x4f567af0L), tole(0x5cf4e287L),
    tole(0x04d43cfdL), tole(0x1776a48aL), tole(0x23910c13L), tole(0x30339464L),
    tole(0x4a5e5d21L), tole(0x59fcc556L), tole(0x6d1b6dcfL), tole(0x7eb9f5b8L),
    tole(0x99c0ff45L), tole(0x8a626732L), tole(0xbe85cfabL), tole(0xad2757dcL),
    tole(0xd74a9e99L), tole(0xc4e806eeL), tole(0xf00fae77L), tole(0xe3ad3600L),
    tole(0x3b11cd7cL), tole(0x28b3550bL), tole(0x1c54fd92L), tole(0x0ff665e5L),
    tole(0x759baca0L), tole(0x663934d7L), tole(0x52de9c4eL), tole(0x417c0439L),
    tole(0xa6050ec4L), tole(0xb5a796b3L), tole(0x81403e2aL), tole(0x92e2a65dL),
    tole(0xe88f6f18L), tole(0xfb2df76fL), tole(0xcfca5ff6L), tole(0xdc68c781L),
    tole(0x7b5fdfffL), tole(0x68fd4788L), tole(0x5c1aef11L), tole(0x4fb87766L),
    tole(0x35d5be23L), tole(0x26772654L), tole(0x12908ecdL), tole(0x013216baL),
    tole(0xe64b1c47L), tole(0xf5e98430L), tole(0xc10e2ca9L), tole(0xd2acb4deL),
    tole(0xa8c17d9bL), tole(0xbb63e5ecL), tole(0x8f844d75L), tole(0x9c26d502L),
    tole(0x449a2e7eL), tole(0x5738b609L), tole(0x63df1e90L), tole(0x707d86e7L),
    tole(0x0a104fa2L), tole(0x19b2d7d5L), tole(0x2d557f4cL), tole(0x3ef7e73bL),
    tole(0xd98eedc6L), tole(0xca2c75b1L), tole(0xfecbdd28L), tole(0xed69455fL),
    tole(0x97048c1aL), tole(0x84a6146dL), tole(0xb041bcf4L), tole(0xa3e32483L)},
    {
    tole(0x00000000L), tole(0xa541927eL), tole(0x4f6f520dL), tole(0xea2ec073L),
    tole(0x9edea41aL), tole(0x3b9f3664L), tole(0xd1b1f617L), tole(0x74f06469L),
    tole(0x38513ec5L), tole(0x9d10acbbL), tole(0x773e6cc8L), tole(0xd27ffeb6L),
    tole(0xa68f9adfL), tole(0x03ce08a1L), tole(0xe9e0c8d2L), tole(0x4ca15aacL),
    tole(0x70a27d8aL), tole(0xd5e3eff4L), tole(0x3fcd2f87L), tole(0x9a8cbdf9L),
    tole(0xee7cd990L), tole(0x4b3d4beeL), tole(0xa1138b9dL), tole(0x045219e3L),
    tole(0x48f3434fL), tole(0xedb2d131L), tole(0x079c1142L), tole(0xa2dd833cL),
    tole(0xd62de755L), tole(0x736c752bL), tole(0x9942b558L), tole(0x3c032726L),
    tole(0xe144fb14L), tole(0x4405696aL), tole(0xae2ba919L), tole(0x0b6a3b67L),
    tole(0x7f9a5f0eL), tole(0xdadbcd70L), tole(0x30f50d03L), tole(0x95b49f7dL),
    tole(0xd915c5d1L), tole(0x7c5457afL), tole(0x967a97dcL), tole(0x333b05a2L),
    tole(0x47cb61cbL), tole(0xe28af3b5L), tole(0x08a433c6L), tole(0xade5a1b8L),
    tole(0x91e6869eL), tole(0x34a714e0L), tole(0xde89d493L), tole(0x7bc846edL),
    tole(0x0f382284L), tole(0xaa79b0faL), tole(0x40577089L), tole(0xe516e2f7L),
    tole(0xa9b7b85bL), tole(0x0cf62a25L), tole(0xe6d8ea56L), tole(0x43997828L),
    tole(0x37691c41L), tole(0x92288e3fL), tole(0x78064e4cL), tole(0xdd47dc32L),
    tole(0xc76580d9L), tole(0x622412a7L), tole(0x880ad2d4L), tole(0x2d4b40aaL),
    tole(0x59bb24c3L), tole(0xfcfab6bdL), tole(0x16d476ceL), tole(0xb395e4b0L),
    tole(0xff34be1cL), tole(0x5a752c62L), tole(0xb05bec11L), tole(0x151a7e6fL),
    tole(0x61ea1a06L), tole(0xc4ab8878L), tole(0x2e85480bL), tole(0x8bc4da75L),
    tole(0xb7c7fd53L), tole(0x12866f2dL), tole(0xf8a8af5eL), tole(0x5de93d20L),
    tole(0x29195949L), tole(0x8c58cb37L), tole(0x66760b44L), tole(0xc337993aL),
    tole(0x8f96c396L), tole(0x2ad751e8L), tole(0xc0f9919bL), tole(0x65b803e5L),
    tole(0x1148678cL), tole(0xb409f5f2L), tole(0x5e273581L), tole(0xfb66a7ffL),
    tole(0x26217bcdL), tole(0x8360e9b3L), tole(0x694e29c0L), tole(0xcc0fbbbeL),
    tole(0xb8ffdfd7L), tole(0x1dbe4da9L), tole(0xf7908ddaL), tole(0x52d11fa4L),
    tole(0x1e704508L), tole(0xbb31d776L), tole(0x511f1705L), tole(0xf45e857bL),
    tole(0x80aee112L), tole(0x25ef736cL), tole(0xcfc1b31fL), tole(0x6a802161L),
    tole(0x56830647L), tole(0xf3c29439L), tole(0x19ec544aL), tole(0xbcadc634L),
    tole(0xc85da25dL), tole(0x6d1c3023L), tole(0x8732f050L), tole(0x2273622eL),
    tole(0x6ed23882L), tole(0xcb93aafcL), tole(0x21bd6a8fL), tole(0x84fcf8f1L),
    tole(0xf00c9c98L), tole(0x554d0ee6L), tole(0xbf63ce95L), tole(0x1a225cebL),
    tole(0x8b277743L), tole(0x2e66e53dL), tole(0xc448254eL), tole(0x6109b730L),
    tole(0x15f9d359L), tole(0xb0b84127L), tole(0x5a968154L), tole(0xffd7132aL),
    tole(0xb3764986L), tole(0x1637dbf8L), tole(0xfc191b8bL), tole(0x595889f5L),
    tole(0x2da8ed9cL), tole(0x88e97fe2L), tole(0x62c7bf91L), tole(0xc7862defL),
    tole(0xfb850ac9L), tole(0x5ec498b7L), tole(0xb4ea58c4L), tole(0x11abcabaL),
    tole(0x655baed3L), tole(0xc01a3cadL), tole(0x2a34fcdeL), tole(0x8f756ea0L),
    tole(0xc3d4340cL), tole(0x6695a672L), tole(0x8cbb6601L), tole(0x29faf47fL),
    tole(0x5d0a9016L), tole(0xf84b0268L), tole(0x1265c21bL), tole(0xb7245065L),
    tole(0x6a638c57L), tole(0xcf221e29L), tole(0x250cde5aL), tole(0x804d4c24L),
    tole(0xf4bd284dL), tole(0x51fcba33L), tole(0xbbd27a40L), tole(0x1e93e83eL),
    tole(0x5232b292L), tole(0xf77320ecL), tole(0x1d5de09fL), tole(0xb81c72e1L),
    tole(0xccec1688L), tole(0x69ad84f6L), tole(0x83834485L), tole(0x26c2d6fbL),
    tole(0x1ac1f1ddL), tole(0xbf8063a3L), tole(0x55aea3d0L), tole(0xf0ef31aeL),
    tole(0x841f55c7L), tole(0x215ec7b9L), tole(0xcb7007caL), tole(0x6e3195b4L),
    tole(0x2290cf18L), tole(0x87d15d66L), tole(0x6dff9d15L), tole(0xc8be0f6bL),
    tole(0xbc4e6b02L), tole(0x190ff97cL), tole(0xf321390fL), tole(0x5660ab71L),
    tole(0x4c42f79aL), tole(0xe90365e4L), tole(0x032da597L), tole(0xa66c37e9L),
    tole(0xd29c5380L), tole(0x77ddc1feL), tole(0x9df3018dL), tole(0x38b293f3L),
    tole(0x7413c95fL), tole(0xd1525b21L), tole(0x3b7c9b52L), tole(0x9e3d092cL),
    tole(0xeacd6d45L), tole(0x4f8cff3bL), tole(0xa5a23f48L), tole(0x00e3ad36L),
    tole(0x3ce08a10L), tole(0x99a1186eL), tole(0x738fd81dL), tole(0xd6ce4a63L),
    tole(0xa23e2e0aL), tole(0x077fbc74L), tole(0xed517c07L), tole(0x4810ee79L),
    tole(0x04b1b4d5L), tole(0xa1f026abL), tole(0x4bdee6d8L), tole(0xee9f74a6L),
    tole(0x9a6f10cfL), tole(0x3f2e82b1L), tole(0xd50042c2L), tole(0x7041d0bcL),
    tole(0xad060c8eL), tole(0x08479ef0L), tole(0xe2695e83L), tole(0x4728ccfdL),
    tole(0x33d8a894L), tole(0x96993aeaL), tole(0x7cb7fa99L), tole(0xd9f668e7L),
    tole(0x9557324bL), tole(0x3016a035L), tole(0xda386046L), tole(0x7f79f238L),
    tole(0x0b899651L), tole(0xaec8042fL), tole(0x44e6c45cL), tole(0xe1a75622L),
    tole(0xdda47104L), tole(0x78e5e37aL), tole(0x92cb2309L), tole(0x378ab177L),
    tole(0x437ad51eL), tole(0xe63b4760L), tole(0x0c158713L), tole(0xa954156dL),
    tole(0xe5f54fc1L), tole(0x40b4ddbfL), tole(0xaa9a1dccL), tole(0x0fdb8fb2L),
    tole(0x7b2bebdbL), tole(0xde6a79a5L), tole(0x3444b9d6L), tole(0x91052ba8L)},
    {
    tole(0x00000000L), tole(0xdd45aab8L), tole(0xbf672381L), tole(0x62228939L),
    tole(0x7b2231f3L), tole(0xa6679b4bL), tole(0xc4451272L), tole(0x1900b8caL),
    tole(0xf64463e6L), tole(0x2b01c95eL), tole(0x49234067L), tole(0x9466eadfL),
    tole(0x8d665215L), tole(0x5023f8adL), tole(0x32017194L), tole(0xef44db2cL),
    tole(0xe964b13dL), tole(0x34211b85L), tole(0x560392bcL), tole(0x8b463804L),
    tole(0x924680ceL), tole(0x4f032a76L), tole(0x2d21a34fL), tole(0xf06409f7L),
    tole(0x1f20d2dbL), tole(0xc2657863L), tole(0xa047f15aL), tole(0x7d025be2L),
    tole(0x6402e328L), tole(0xb9474990L), tole(0xdb65c0a9L), tole(0x06206a11L),
    tole(0xd725148bL), tole(0x0a60be33L), tole(0x6842370aL), tole(0xb5079db2L),
    tole(0xac072578L), tole(0x71428fc0L), tole(0x136006f9L), tole(0xce25ac41L),
    tole(0x2161776dL), tole(0xfc24ddd5L), tole(0x9e0654ecL), tole(0x4343fe54L),
    tole(0x5a43469eL), tole(0x8706ec26L), tole(0xe524651fL), tole(0x3861cfa7L),
    tole(0x3e41a5b6L), tole(0xe3040f0eL), tole(0x81268637L), tole(0x5c632c8fL),
    tole(0x45639445L), tole(0x98263efdL), tole(0xfa04b7c4L), tole(0x27411d7cL),
    tole(0xc805c650L), tole(0x15406ce8L), tole(0x7762e5d1L), tole(0xaa274f69L),
    tole(0xb327f7a3L), tole(0x6e625d1bL), tole(0x0c40d422L), tole(0xd1057e9aL),
    tole(0xaba65fe7L), tole(0x76e3f55fL), tole(0x14c17c66L), tole(0xc984d6deL),
    tole(0xd0846e14L), tole(0x0dc1c4acL), tole(0x6fe34d95L), tole(0xb2a6e72dL),
    tole(0x5de23c01L), tole(0x80a796b9L), tole(0xe2851f80L), tole(0x3fc0b538L),
    tole(0x26c00df2L), tole(0xfb85a74aL), tole(0x99a72e73L), tole(0x44e284cbL),
    tole(0x42c2eedaL), tole(0x9f874462L), tole(0xfda5cd5bL), tole(0x20e067e3L),
    tole(0x39e0df29L), tole(0xe4a57591L), tole(0x8687fca8L), tole(0x5bc25610L),
    tole(0xb4868d3cL), tole(0x69c32784L), tole(0x0be1aebdL), tole(0xd6a40405L),
    tole(0xcfa4bccfL), tole(0x12e11677L), tole(0x70c39f4eL), tole(0xad8635f6L),
    tole(0x7c834b6cL), tole(0xa1c6e1d4L), tole(0xc3e468edL), tole(0x1ea1c255L),
    tole(0x07a17a9fL), tole(0xdae4d027L), tole(0xb8c6591eL), tole(0x6583f3a6L),
    tole(0x8ac7288aL), tole(0x57828232L), tole(0x35a00b0bL), tole(0xe8e5a1b3L),
    tole(0xf1e51979L), tole(0x2ca0b3c1L), tole(0x4e823af8L), tole(0x93c79040L),
    tole(0x95e7fa51L), tole(0x48a250e9L), tole(0x2a80d9d0L), tole(0xf7c57368L),
    tole(0xeec5cba2L), tole(0x3380611aL), tole(0x51a2e823L), tole(0x8ce7429bL),
    tole(0x63a399b7L), tole(0xbee6330fL), tole(0xdcc4ba36L), tole(0x0181108eL),
    tole(0x1881a844L), tole(0xc5c402fcL), tole(0xa7e68bc5L), tole(0x7aa3217dL),
    tole(0x52a0c93fL), tole(0x8fe56387L), tole(0xedc7eabeL), tole(0x30824006L),
    tole(0x2982f8ccL), tole(0xf4c75274L), tole(0x96e5db4dL), tole(0x4ba071f5L),
    tole(0xa4e4aad9L), tole(0x79a10061L), tole(0x1b838958L), tole(0xc6c623e0L),
    tole(0xdfc69b2aL), tole(0x02833192L), tole(0x60a1b8abL), tole(0xbde41213L),
    tole(0xbbc47802L), tole(0x6681d2baL), tole(0x04a35b83L), tole(0xd9e6f13bL),
    tole(0xc0e649f1L), tole(0x1da3e349L), tole(0x7f816a70L), tole(0xa2c4c0c8L),
    tole(0x4d801be4L), tole(0x90c5b15cL), tole(0xf2e73865L), tole(0x2fa292ddL),
    tole(0x36a22a17L), tole(0xebe780afL), tole(0x89c50996L), tole(0x5480a32eL),
    tole(0x8585ddb4L), tole(0x58c0770cL), tole(0x3ae2fe35L), tole(0xe7a7548dL),
    tole(0xfea7ec47L), tole(0x23e246ffL), tole(0x41c0cfc6L), tole(0x9c85657eL),
    tole(0x73c1be52L), tole(0xae8414eaL), tole(0xcca69dd3L), tole(0x11e3376bL),
    tole(0x08e38fa1L), tole(0xd5a62519L), tole(0xb784ac20L), tole(0x6ac10698L),
    tole(0x6ce16c89L), tole(0xb1a4c631L), tole(0xd3864f08L), tole(0x0ec3e5b0L),
    tole(0x17c35d7aL), tole(0xca86f7c2L), tole(0xa8a47efbL), tole(0x75e1d443L),
    tole(0x9aa50f6fL), tole(0x47e0a5d7L), tole(0x25c22ceeL), tole(0xf8878656L),
    tole(0xe1873e9cL), tole(0x3cc29424L), tole(0x5ee01d1dL), tole(0x83a5b7a5L),
    tole(0xf90696d8L), tole(0x24433c60L), tole(0x4661b559L), tole(0x9b241fe1L),
    tole(0x8224a72bL), tole(0x5f610d93L), tole(0x3d4384aaL), tole(0xe0062e12L),
    tole(0x0f42f53eL), tole(0xd2075f86L), tole(0xb025d6bfL), tole(0x6d607c07L),
    tole(0x7460c4cdL), tole(0xa9256e75L), tole(0xcb07e74cL), tole(0x16424df4L),
    tole(0x106227e5L), tole(0xcd278d5dL), tole(0xaf050464L), tole(0x7240aedcL),
    tole(0x6b401616L), tole(0xb605bcaeL), tole(0xd4273597L), tole(0x09629f2fL),
    tole(0xe6264403L), tole(0x3b63eebbL), tole(0x59416782L), tole(0x8404cd3aL),
    tole(0x9d0475f0L), tole(0x4041df48L), tole(0x22635671L), tole(0xff26fcc9L),
    tole(0x2e238253L), tole(0xf36628ebL), tole(0x9144a1d2L), tole(0x4c010b6aL),
    tole(0x5501b3a0L), tole(0x88441918L), tole(0xea669021L), tole(0x37233a99L),
    tole(0xd867e1b5L), tole(0x05224b0dL), tole(0x6700c234L), tole(0xba45688cL),
    tole(0xa345d046L), tole(0x7e007afeL), tole(0x1c22f3c7L), tole(0xc167597fL),
    tole(0xc747336eL), tole(0x1a0299d6L), tole(0x782010efL), tole(0xa565ba57L),
    tole(0xbc65029dL), tole(0x6120a825L), tole(0x0302211cL), tole(0xde478ba4L),
    tole(0x31035088L), tole(0xec46fa30L), tole(0x8e647309L), tole(0x5321d9b1L),
    tole(0x4a21617bL), tole(0x9764cbc3L), tole(0xf54642faL), tole(0x2803e842L)},
    {
    tole(0x00000000L), tole(0x38116facL), tole(0x7022df58L), tole(0x4833b0f4L),
    tole(0xe045beb0L), tole(0xd854d11cL), tole(0x906761e8L), tole(0xa8760e44L),
    tole(0xc5670b91L), tole(0xfd76643dL), tole(0xb545d4c9L), tole(0x8d54bb65L),
    tole(0x2522b521L), tole(0x1d33da8dL), tole(0x55006a79L), tole(0x6d1105d5L),
    tole(0x8f2261d3L), tole(0xb7330e7fL), tole(0xff00be8bL), tole(0xc711d127L),
    tole(0x6f67df63L), tole(0x5776b0cfL), tole(0x1f45003bL), tole(0x27546f97L),
    tole(0x4a456a42L), tole(0x725405eeL), tole(0x3a67b51aL), tole(0x0276dab6L),
    tole(0xaa00d4f2L), tole(0x9211bb5eL), tole(0xda220baaL), tole(0xe2336406L),
    tole(0x1ba8b557L), tole(0x23b9dafbL), tole(0x6b8a6a0fL), tole(0x539b05a3L),
    tole(0xfbed0be7L), tole(0xc3fc644bL), tole(0x8bcfd4bfL), tole(0xb3debb13L),
    tole(0xdecfbec6L), tole(0xe6ded16aL), tole(0xaeed619eL), tole(0x96fc0e32L),
    tole(0x3e8a0076L), tole(0x069b6fdaL), tole(0x4ea8df2eL), tole(0x76b9b082L),
    tole(0x948ad484L), tole(0xac9bbb28L), tole(0xe4a80bdcL), tole(0xdcb96470L),
    tole(0x74cf6a34L), tole(0x4cde0598L), tole(0x04edb56cL), tole(0x3cfcdac0L),
    tole(0x51eddf15L), tole(0x69fcb0b9L), tole(0x21cf004dL), tole(0x19de6fe1L),
    tole(0xb1a861a5L), tole(0x89b90e09L), tole(0xc18abefdL), tole(0xf99bd151L),
    tole(0x37516aaeL), tole(0x0f400502L), tole(0x4773b5f6L), tole(0x7f62da5aL),
    tole(0xd714d41eL), tole(0xef05bbb2L), tole(0xa7360b46L), tole(0x9f2764eaL),
    tole(0xf236613fL), tole(0xca270e93L), tole(0x8214be67L), tole(0xba05d1cbL),
    tole(0x1273df8fL), tole(0x2a62b023L), tole(0x625100d7L), tole(0x5a406f7bL),
    tole(0xb8730b7dL), tole(0x806264d1L), tole(0xc851d425L), tole(0xf040bb89L),
    tole(0x5836b5cdL), tole(0x6027da61L), tole(0x28146a95L), tole(0x10050539L),
    tole(0x7d1400ecL), tole(0x45056f40L), tole(0x0d36dfb4L), tole(0x3527b018L),
    tole(0x9d51be5cL), tole(0xa540d1f0L), tole(0xed736104L), tole(0xd5620ea8L),
    tole(0x2cf9dff9L), tole(0x14e8b055L), tole(0x5cdb00a1L), tole(0x64ca6f0dL),
    tole(0xccbc6149L), tole(0xf4ad0ee5L), tole(0xbc9ebe11L), tole(0x848fd1bdL),
    tole(0xe99ed468L), tole(0xd18fbbc4L), tole(0x99bc0b30L), tole(0xa1ad649cL),
    tole(0x09db6ad8L), tole(0x31ca0574L), tole(0x79f9b580L), tole(0x41e8da2cL),
    tole(0xa3dbbe2aL), tole(0x9bcad186L), tole(0xd3f96172L), tole(0xebe80edeL),
    tole(0x439e009aL), tole(0x7b8f6f36L), tole(0x33bcdfc2L), tole(0x0badb06eL),
    tole(0x66bcb5bbL), tole(0x5eadda17L), tole(0x169e6ae3L), tole(0x2e8f054fL),
    tole(0x86f90b0bL), tole(0xbee864a7L), tole(0xf6dbd453L), tole(0xcecabbffL),
    tole(0x6ea2d55cL), tole(0x56b3baf0L), tole(0x1e800a04L), tole(0x269165a8L),
    tole(0x8ee76becL), tole(0xb6f60440L), tole(0xfec5b4b4L), tole(0xc6d4db18L),
    tole(0xabc5decdL), tole(0x93d4b161L), tole(0xdbe70195L), tole(0xe3f66e39L),
    tole(0x4b80607dL), tole(0x73910fd1L), tole(0x3ba2bf25L), tole(0x03b3d089L),
    tole(0xe180b48fL), tole(0xd991db23L), tole(0x91a26bd7L), tole(0xa9b3047bL),
    tole(0x01c50a3fL), tole(0x39d46593L), tole(0x71e7d567L), tole(0x49f6bacbL),
    tole(0x24e7bf1eL), tole(0x1cf6d0b2L), tole(0x54c56046L), tole(0x6cd40feaL),
    tole(0xc4a201aeL), tole(0xfcb36e02L), tole(0xb480def6L), tole(0x8c91b15aL),
    tole(0x750a600bL), tole(0x4d1b0fa7L), tole(0x0528bf53L), tole(0x3d39d0ffL),
    tole(0x954fdebbL), tole(0xad5eb117L), tole(0xe56d01e3L), tole(0xdd7c6e4fL),
    tole(0xb06d6b9aL), tole(0x887c0436L), tole(0xc04fb4c2L), tole(0xf85edb6eL),
    tole(0x5028d52aL), tole(0x6839ba86L), tole(0x200a0a72L), tole(0x181b65deL),
    tole(0xfa2801d8L), tole(0xc2396e74L), tole(0x8a0ade80L), tole(0xb21bb12cL),
    tole(0x1a6dbf68L), tole(0x227cd0c4L), tole(0x6a4f6030L), tole(0x525e0f9cL),
    tole(0x3f4f0a49L), tole(0x075e65e5L), tole(0x4f6dd511L), tole(0x777cbabdL),
    tole(0xdf0ab4f9L), tole(0xe71bdb55L), tole(0xaf286ba1L), tole(0x9739040dL),
    tole(0x59f3bff2L), tole(0x61e2d05eL), tole(0x29d160aaL), tole(0x11c00f06L),
    tole(0xb9b60142L), tole(0x81a76eeeL), tole(0xc994de1aL), tole(0xf185b1b6L),
    tole(0x9c94b463L), tole(0xa485dbcfL), tole(0xecb66b3bL), tole(0xd4a70497L),
    tole(0x7cd10ad3L), tole(0x44c0657fL), tole(0x0cf3d58bL), tole(0x34e2ba27L),
    tole(0xd6d1de21L), tole(0xeec0b18dL), tole(0xa6f30179L), tole(0x9ee26ed5L),
    tole(0x36946091L), tole(0x0e850f3dL), tole(0x46b6bfc9L), tole(0x7ea7d065L),
    tole(0x13b6d5b0L), tole(0x2ba7ba1cL), tole(0x63940ae8L), tole(0x5b856544L),
    tole(0xf3f36b00L), tole(0xcbe204acL), tole(0x83d1b458L), tole(0xbbc0dbf4L),
    tole(0x425b0aa5L), tole(0x7a4a6509L), tole(0x3279d5fdL), tole(0x0a68ba51L),
    tole(0xa21eb415L), tole(0x9a0fdbb9L), tole(0xd23c6b4dL), tole(0xea2d04e1L),
    tole(0x873c0134L), tole(0xbf2d6e98L), tole(0xf71ede6cL), tole(0xcf0fb1c0L),
    tole(0x6779bf84L), tole(0x5f68d028L), tole(0x175b60dcL), tole(0x2f4a0f70L),
    tole(0xcd796b76L), tole(0xf56804daL), tole(0xbd5bb42eL), tole(0x854adb82L),
    tole(0x2d3cd5c6L), tole(0x152dba6aL), tole(0x5d1e0a9eL), tole(0x650f6532L),
    tole(0x081e60e7L), tole(0x300f0f4bL), tole(0x783cbfbfL), tole(0x402dd013L),
    tole(0xe85bde57L), tole(0xd04ab1fbL), tole(0x9879010fL), tole(0xa0686ea3L)},
    {
    tole(0x00000000L), tole(0xef306b19L), tole(0xdb8ca0c3L), tole(0x34bccbdaL),
    tole(0xb2f53777L), tole(0x5dc55c6eL), tole(0x697997b4L), tole(0x8649fcadL),
    tole(0x6006181fL), tole(0x8f367306L), tole(0xbb8ab8dcL), tole(0x54bad3c5L),
    tole(0xd2f32f68L), tole(0x3dc34471L), tole(0x097f8fabL), tole(0xe64fe4b2L),
    tole(0xc00c303eL), tole(0x2f3c5b27L), tole(0x1b8090fdL), tole(0xf4b0fbe4L),
    tole(0x72f90749L), tole(0x9dc96c50L), tole(0xa975a78aL), tole(0x4645cc93L),
    tole(0xa00a2821L), tole(0x4f3a4338L), tole(0x7b8688e2L), tole(0x94b6e3fbL),
    tole(0x12ff1f56L), tole(0xfdcf744fL), tole(0xc973bf95L), tole(0x2643d48cL),
    tole(0x85f4168dL), tole(0x6ac47d94L), tole(0x5e78b64eL), tole(0xb148dd57L),
    tole(0x370121faL), tole(0xd8314ae3L), tole(0xec8d8139L), tole(0x03bdea20L),
    tole(0xe5f20e92L), tole(0x0ac2658bL), tole(0x3e7eae51L), tole(0xd14ec548L),
    tole(0x570739e5L), tole(0xb83752fcL), tole(0x8c8b9926L), tole(0x63bbf23fL),
    tole(0x45f826b3L), tole(0xaac84daaL), tole(0x9e748670L), tole(0x7144ed69L),
    tole(0xf70d11c4L), tole(0x183d7addL), tole(0x2c81b107L), tole(0xc3b1da1eL),
    tole(0x25fe3eacL), tole(0xcace55b5L), tole(0xfe729e6fL), tole(0x1142f576L),
    tole(0x970b09dbL), tole(0x783b62c2L), tole(0x4c87a918L), tole(0xa3b7c201L),
    tole(0x0e045bebL), tole(0xe13430f2L), tole(0xd588fb28L), tole(0x3ab89031L),
    tole(0xbcf16c9cL), tole(0x53c10785L), tole(0x677dcc5fL), tole(0x884da746L),
    tole(0x6e0243f4L), tole(0x813228edL), tole(0xb58ee337L), tole(0x5abe882eL),
    tole(0xdcf77483L), tole(0x33c71f9aL), tole(0x077bd440L), tole(0xe84bbf59L),
    tole(0xce086bd5L), tole(0x213800ccL), tole(0x1584cb16L), tole(0xfab4a00fL),
    tole(0x7cfd5ca2L), tole(0x93cd37bbL), tole(0xa771fc61L), tole(0x48419778L),
    tole(0xae0e73caL), tole(0x413e18d3L), tole(0x7582d309L), tole(0x9ab2b810L),
    tole(0x1cfb44bdL), tole(0xf3cb2fa4L), tole(0xc777e47eL), tole(0x28478f67L),
    tole(0x8bf04d66L), tole(0x64c0267fL), tole(0x507ceda5L), tole(0xbf4c86bcL),
    tole(0x39057a11L), tole(0xd6351108L), tole(0xe289dad2L), tole(0x0db9b1cbL),
    tole(0xebf65579L), tole(0x04c63e60L), tole(0x307af5baL), tole(0xdf4a9ea3L),
    tole(0x5903620eL), tole(0xb6330917L), tole(0x828fc2cdL), tole(0x6dbfa9d4L),
    tole(0x4bfc7d58L), tole(0xa4cc1641L), tole(0x9070dd9bL), tole(0x7f40b682L),
    tole(0xf9094a2fL), tole(0x16392136L), tole(0x2285eaecL), tole(0xcdb581f5L),
    tole(0x2bfa6547L), tole(0xc4ca0e5eL), tole(0xf076c584L), tole(0x1f46ae9dL),
    tole(0x990f5230L), tole(0x763f3929L), tole(0x4283f2f3L), tole(0xadb399eaL),
    tole(0x1c08b7d6L), tole(0xf338dccfL), tole(0xc7841715L), tole(0x28b47c0cL),
    tole(0xaefd80a1L), tole(0x41cdebb8L), tole(0x75712062L), tole(0x9a414b7bL),
    tole(0x7c0eafc9L), tole(0x933ec4d0L), tole(0xa7820f0aL), tole(0x48b26413L),
    tole(0xcefb98beL), tole(0x21cbf3a7L), tole(0x1577387dL), tole(0xfa475364L),
    tole(0xdc0487e8L), tole(0x3334ecf1L), tole(0x0788272bL), tole(0xe8b84c32L),
    tole(0x6ef1b09fL), tole(0x81c1db86L), tole(0xb57d105cL), tole(0x5a4d7b45L),
    tole(0xbc029ff7L), tole(0x5332f4eeL), tole(0x678e3f34L), tole(0x88be542dL),
    tole(0x0ef7a880L), tole(0xe1c7c399L), tole(0xd57b0843L), tole(0x3a4b635aL),
    tole(0x99fca15bL), tole(0x76ccca42L), tole(0x42700198L), tole(0xad406a81L),
    tole(0x2b09962cL), tole(0xc439fd35L), tole(0xf08536efL), tole(0x1fb55df6L),
    tole(0xf9fab944L), tole(0x16cad25dL), tole(0x22761987L), tole(0xcd46729eL),
    tole(0x4b0f8e33L), tole(0xa43fe52aL), tole(0x90832ef0L), tole(0x7fb345e9L),
    tole(0x59f09165L), tole(0xb6c0fa7cL), tole(0x827c31a6L), tole(0x6d4c5abfL),
    tole(0xeb05a612L), tole(0x0435cd0bL), tole(0x308906d1L), tole(0xdfb96dc8L),
    tole(0x39f6897aL), tole(0xd6c6e263L), tole(0xe27a29b9L), tole(0x0d4a42a0L),
    tole(0x8b03be0dL), tole(0x6433d514L), tole(0x508f1eceL), tole(0xbfbf75d7L),
    tole(0x120cec3dL), tole(0xfd3c8724L), tole(0xc9804cfeL), tole(0x26b027e7L),
    tole(0xa0f9db4aL), tole(0x4fc9b053L), tole(0x7b757b89L), tole(0x94451090L),
    tole(0x720af422L), tole(0x9d3a9f3bL), tole(0xa98654e1L), tole(0x46b63ff8L),
    tole(0xc0ffc355L), tole(0x2fcfa84cL), tole(0x1b736396L), tole(0xf443088fL),
    tole(0xd200dc03L), tole(0x3d30b71aL), tole(0x098c7cc0L), tole(0xe6bc17d9L),
    tole(0x60f5eb74L), tole(0x8fc5806dL), tole(0xbb794bb7L), tole(0x544920aeL),
    tole(0xb206c41cL), tole(0x5d36af05L), tole(0x698a64dfL), tole(0x86ba0fc6L),
    tole(0x00f3f36bL), tole(0xefc39872L), tole(0xdb7f53a8L), tole(0x344f38b1L),
    tole(0x97f8fab0L), tole(0x78c891a9L), tole(0x4c745a73L), tole(0xa344316aL),
    tole(0x250dcdc7L), tole(0xca3da6deL), tole(0xfe816d04L), tole(0x11b1061dL),
    tole(0xf7fee2afL), tole(0x18ce89b6L), tole(0x2c72426cL), tole(0xc3422975L),
    tole(0x450bd5d8L), tole(0xaa3bbec1L), tole(0x9e87751bL), tole(0x71b71e02L),
    tole(0x57f4ca8eL), tole(0xb8c4a197L), tole(0x8c786a4dL), tole(0x63480154L),
    tole(0xe501fdf9L), tole(0x0a3196e0L), tole(0x3e8d5d3aL), tole(0xd1bd3623L),
    tole(0x37f2d291L), tole(0xd8c2b988L), tole(0xec7e7252L), tole(0x034e194bL),
    tole(0x8507e5e6L), tole(0x6a378effL), tole(0x5e8b4525L), tole(0xb1bb2e3cL)},
    {
    tole(0x00000000L), tole(0x68032cc8L), tole(0xd0065990L), tole(0xb8057558L),
    tole(0xa5e0c5d1L), tole(0xcde3e919L), tole(0x75e69c41L), tole(0x1de5b089L),
    tole(0x4e2dfd53L), tole(0x262ed19bL), tole(0x9e2ba4c3L), tole(0xf628880bL),
    tole(0xebcd3882L), tole(0x83ce144aL), tole(0x3bcb6112L), tole(0x53c84ddaL),
    tole(0x9c5bfaa6L), tole(0xf458d66eL), tole(0x4c5da336L), tole(0x245e8ffeL),
    tole(0x39bb3f77L), tole(0x51b813bfL), tole(0xe9bd66e7L), tole(0x81be4a2fL),
    tole(0xd27607f5L), tole(0xba752b3dL), tole(0x02705e65L), tole(0x6a7372adL),
    tole(0x7796c224L), tole(0x1f95eeecL), tole(0xa7909bb4L), tole(0xcf93b77cL),
    tole(0x3d5b83bdL), tole(0x5558af75L), tole(0xed5dda2dL), tole(0x855ef6e5L),
    tole(0x98bb466cL), tole(0xf0b86aa4L), tole(0x48bd1ffcL), tole(0x20be3334L),
    tole(0x73767eeeL), tole(0x1b755226L), tole(0xa370277eL), tole(0xcb730bb6L),
    tole(0xd696bb3fL), tole(0xbe9597f7L), tole(0x0690e2afL), tole(0x6e93ce67L),
    tole(0xa100791bL), tole(0xc90355d3L), tole(0x7106208bL), tole(0x19050c43L),
    tole(0x04e0bccaL), tole(0x6ce39002L), tole(0xd4e6e55aL), tole(0xbce5c992L),
    tole(0xef2d8448L), tole(0x872ea880L), tole(0x3f2bddd8L), tole(0x5728f110L),
    tole(0x4acd4199L), tole(0x22ce6d51L), tole(0x9acb1809L), tole(0xf2c834c1L),
    tole(0x7ab7077aL), tole(0x12b42bb2L), tole(0xaab15eeaL), tole(0xc2b27222L),
    tole(0xdf57c2abL), tole(0xb754ee63L), tole(0x0f519b3bL), tole(0x6752b7f3L),
    tole(0x349afa29L), tole(0x5c99d6e1L), tole(0xe49ca3b9L), tole(0x8c9f8f71L),
    tole(0x917a3ff8L), tole(0xf9791330L), tole(0x417c6668L), tole(0x297f4aa0L),
    tole(0xe6ecfddcL), tole(0x8eefd114L), tole(0x36eaa44cL), tole(0x5ee98884L),
    tole(0x430c380dL), tole(0x2b0f14c5L), tole(0x930a619dL), tole(0xfb094d55L),
    tole(0xa8c1008fL), tole(0xc0c22c47L), tole(0x78c7591fL), tole(0x10c475d7L),
    tole(0x0d21c55eL), tole(0x6522e996L), tole(0xdd279cceL), tole(0xb524b006L),
    tole(0x47ec84c7L), tole(0x2fefa80fL), tole(0x97eadd57L), tole(0xffe9f19fL),
    tole(0xe20c4116L), tole(0x8a0f6ddeL), tole(0x320a1886L), tole(0x5a09344eL),
    tole(0x09c17994L), tole(0x61c2555cL), tole(0xd9c72004L), tole(0xb1c40cccL),
    tole(0xac21bc45L), tole(0xc422908dL), tole(0x7c27e5d5L), tole(0x1424c91dL),
    tole(0xdbb77e61L), tole(0xb3b452a9L), tole(0x0bb127f1L), tole(0x63b20b39L),
    tole(0x7e57bbb0L), tole(0x16549778L), tole(0xae51e220L), tole(0xc652cee8L),
    tole(0x959a8332L), tole(0xfd99affaL), tole(0x459cdaa2L), tole(0x2d9ff66aL),
    tole(0x307a46e3L), tole(0x58796a2bL), tole(0xe07c1f73L), tole(0x887f33bbL),
    tole(0xf56e0ef4L), tole(0x9d6d223cL), tole(0x25685764L), tole(0x4d6b7bacL),
    tole(0x508ecb25L), tole(0x388de7edL), tole(0x808892b5L), tole(0xe88bbe7dL),
    tole(0xbb43f3a7L), tole(0xd340df6fL), tole(0x6b45aa37L), tole(0x034686ffL),
    tole(0x1ea33676L), tole(0x76a01abeL), tole(0xcea56fe6L), tole(0xa6a6432eL),
    tole(0x6935f452L), tole(0x0136d89aL), tole(0xb933adc2L), tole(0xd130810aL),
    tole(0xccd53183L), tole(0xa4d61d4bL), tole(0x1cd36813L), tole(0x74d044dbL),
    tole(0x27180901L), tole(0x4f1b25c9L), tole(0xf71e5091L), tole(0x9f1d7c59L),
    tole(0x82f8ccd0L), tole(0xeafbe018L), tole(0x52fe9540L), tole(0x3afdb988L),
    tole(0xc8358d49L), tole(0xa036a181L), tole(0x1833d4d9L), tole(0x7030f811L),
    tole(0x6dd54898L), tole(0x05d66450L), tole(0xbdd31108L), tole(0xd5d03dc0L),
    tole(0x8618701aL), tole(0xee1b5cd2L), tole(0x561e298aL), tole(0x3e1d0542L),
    tole(0x23f8b5cbL), tole(0x4bfb9903L), tole(0xf3feec5bL), tole(0x9bfdc093L),
    tole(0x546e77efL), tole(0x3c6d5b27L), tole(0x84682e7fL), tole(0xec6b02b7L),
    tole(0xf18eb23eL), tole(0x998d9ef6L), tole(0x2188ebaeL), tole(0x498bc766L),
    tole(0x1a438abcL), tole(0x7240a674L), tole(0xca45d32cL), tole(0xa246ffe4L),
    tole(0xbfa34f6dL), tole(0xd7a063a5L), tole(0x6fa516fdL), tole(0x07a63a35L),
    tole(0x8fd9098eL), tole(0xe7da2546L), tole(0x5fdf501eL), tole(0x37dc7cd6L),
    tole(0x2a39cc5fL), tole(0x423ae097L), tole(0xfa3f95cfL), tole(0x923cb907L),
    tole(0xc1f4f4ddL), tole(0xa9f7d815L), tole(0x11f2ad4dL), tole(0x79f18185L),
    tole(0x6414310cL), tole(0x0c171dc4L), tole(0xb412689cL), tole(0xdc114454L),
    tole(0x1382f328L), tole(0x7b81dfe0L), tole(0xc384aab8L), tole(0xab878670L),
    tole(0xb66236f9L), tole(0xde611a31L), tole(0x66646f69L), tole(0x0e6743a1L),
    tole(0x5daf0e7bL), tole(0x35ac22b3L), tole(0x8da957ebL), tole(0xe5aa7b23L),
    tole(0xf84fcbaaL), tole(0x904ce762L), tole(0x2849923aL), tole(0x404abef2L),
    tole(0xb2828a33L), tole(0xda81a6fbL), tole(0x6284d3a3L), tole(0x0a87ff6bL),
    tole(0x17624fe2L), tole(0x7f61632aL), tole(0xc7641672L), tole(0xaf673abaL),
    tole(0xfcaf7760L), tole(0x94ac5ba8L), tole(0x2ca92ef0L), tole(0x44aa0238L),
    tole(0x594fb2b1L), tole(0x314c9e79L), tole(0x8949eb21L), tole(0xe14ac7e9L),
    tole(0x2ed97095L), tole(0x46da5c5dL), tole(0xfedf2905L), tole(0x96dc05cdL),
    tole(0x8b39b544L), tole(0xe33a998cL), tole(0x5b3fecd4L), tole(0x333cc01cL),
    tole(0x60f48dc6L), tole(0x08f7a10eL), tole(0xb0f2d456L), tole(0xd8f1f89eL),
    tole(0xc5144817L), tole(0xad1764dfL), tole(0x15121187L), tole(0x7d113d4fL)},
    {
    tole(0x00000000L), tole(0x493c7d27L), tole(0x9278fa4eL), tole(0xdb448769L),
    tole(0x211d826dL), tole(0x6821ff4aL), tole(0xb3657823L), tole(0xfa590504L),
    tole(0x423b04daL), tole(0x0b0779fdL), tole(0xd043fe94L), tole(0x997f83b3L),
    tole(0x632686b7L), tole(0x2a1afb90L), tole(0xf15e7cf9L), tole(0xb86201deL),
    tole(0x847609b4L), tole(0xcd4a7493L), tole(0x160ef3faL), tole(0x5f328eddL),
    tole(0xa56b8bd9L), tole(0xec57f6feL), tole(0x37137197L), tole(0x7e2f0cb0L),
    tole(0xc64d0d6eL), tole(0x8f717049L), tole(0x5435f720L), tole(0x1d098a07L),
    tole(0xe7508f03L), tole(0xae6cf224L), tole(0x7528754dL), tole(0x3c14086aL),
    tole(0x0d006599L), tole(0x443c18beL), tole(0x9f789fd7L), tole(0xd644e2f0L),
    tole(0x2c1de7f4L), tole(0x65219ad3L), tole(0xbe651dbaL), tole(0xf759609dL),
    tole(0x4f3b6143L), tole(0x06071c64L), tole(0xdd439b0dL), tole(0x947fe62aL),
    tole(0x6e26e32eL), tole(0x271a9e09L), tole(0xfc5e1960L), tole(0xb5626447L),
    tole(0x89766c2dL), tole(0xc04a110aL), tole(0x1b0e9663L), tole(0x5232eb44L),
    tole(0xa86bee40L), tole(0xe1579367L), tole(0x3a13140eL), tole(0x732f6929L),
    tole(0xcb4d68f7L), tole(0x827115d0L), tole(0x593592b9L), tole(0x1009ef9eL),
    tole(0xea50ea9aL), tole(0xa36c97bdL), tole(0x782810d4L), tole(0x31146df3L),
    tole(0x1a00cb32L), tole(0x533cb615L), tole(0x8878317cL), tole(0xc1444c5bL),
    tole(0x3b1d495fL), tole(0x72213478L), tole(0xa965b311L), tole(0xe059ce36L),
    tole(0x583bcfe8L), tole(0x1107b2cfL), tole(0xca4335a6L), tole(0x837f4881L),
    tole(0x79264d85L), tole(0x301a30a2L), tole(0xeb5eb7cbL), tole(0xa262caecL),
    tole(0x9e76c286L), tole(0xd74abfa1L), tole(0x0c0e38c8L), tole(0x453245efL),
    tole(0xbf6b40ebL), tole(0xf6573dccL), tole(0x2d13baa5L), tole(0x642fc782L),
    tole(0xdc4dc65cL), tole(0x9571bb7bL), tole(0x4e353c12L), tole(0x07094135L),
    tole(0xfd504431L), tole(0xb46c3916L), tole(0x6f28be7fL), tole(0x2614c358L),
    tole(0x1700aeabL), tole(0x5e3cd38cL), tole(0x857854e5L), tole(0xcc4429c2L),
    tole(0x361d2cc6L), tole(0x7f2151e1L), tole(0xa465d688L), tole(0xed59abafL),
    tole(0x553baa71L), tole(0x1c07d756L), tole(0xc743503fL), tole(0x8e7f2d18L),
    tole(0x7426281cL), tole(0x3d1a553bL), tole(0xe65ed252L), tole(0xaf62af75L),
    tole(0x9376a71fL), tole(0xda4ada38L), tole(0x010e5d51L), tole(0x48322076L),
    tole(0xb26b2572L), tole(0xfb575855L), tole(0x2013df3cL), tole(0x692fa21bL),
    tole(0xd14da3c5L), tole(0x9871dee2L), tole(0x4335598bL), tole(0x0a0924acL),
    tole(0xf05021a8L), tole(0xb96c5c8fL), tole(0x6228dbe6L), tole(0x2b14a6c1L),
    tole(0x34019664L), tole(0x7d3deb43L), tole(0xa6796c2aL), tole(0xef45110dL),
    tole(0x151c1409L), tole(0x5c20692eL), tole(0x8764ee47L), tole(0xce589360L),
    tole(0x763a92beL), tole(0x3f06ef99L), tole(0xe44268f0L), tole(0xad7e15d7L),
    tole(0x572710d3L), tole(0x1e1b6df4L), tole(0xc55fea9dL), tole(0x8c6397baL),
    tole(0xb0779fd0L), tole(0xf94be2f7L), tole(0x220f659eL), tole(0x6b3318b9L),
    tole(0x916a1dbdL), tole(0xd856609aL), tole(0x0312e7f3L), tole(0x4a2e9ad4L),
    tole(0xf24c9b0aL), tole(0xbb70e62dL), tole(0x60346144L), tole(0x29081c63L),
    tole(0xd3511967L), tole(0x9a6d6440L), tole(0x4129e329L), tole(0x08159e0eL),
    tole(0x3901f3fdL), tole(0x703d8edaL), tole(0xab7909b3L), tole(0xe2457494L),
    tole(0x181c7190L), tole(0x51200cb7L), tole(0x8a648bdeL), tole(0xc358f6f9L),
    tole(0x7b3af727L), tole(0x32068a00L), tole(0xe9420d69L), tole(0xa07e704eL),
    tole(0x5a27754aL), tole(0x131b086dL), tole(0xc85f8f04L), tole(0x8163f223L),
    tole(0xbd77fa49L), tole(0xf44b876eL), tole(0x2f0f0007L), tole(0x66337d20L),
    tole(0x9c6a7824L), tole(0xd5560503L), tole(0x0e12826aL), tole(0x472eff4dL),
    tole(0xff4cfe93L), tole(0xb67083b4L), tole(0x6d3404ddL), tole(0x240879faL),
    tole(0xde517cfeL), tole(0x976d01d9L), tole(0x4c2986b0L), tole(0x0515fb97L),
    tole(0x2e015d56L), tole(0x673d2071L), tole(0xbc79a718L), tole(0xf545da3fL),
    tole(0x0f1cdf3bL), tole(0x4620a21cL), tole(0x9d642575L), tole(0xd4585852L),
    tole(0x6c3a598cL), tole(0x250624abL), tole(0xfe42a3c2L), tole(0xb77edee5L),
    tole(0x4d27dbe1L), tole(0x041ba6c6L), tole(0xdf5f21afL), tole(0x96635c88L),
    tole(0xaa7754e2L), tole(0xe34b29c5L), tole(0x380faeacL), tole(0x7133d38bL),
    tole(0x8b6ad68fL), tole(0xc256aba8L), tole(0x19122cc1L), tole(0x502e51e6L),
    tole(0xe84c5038L), tole(0xa1702d1fL), tole(0x7a34aa76L), tole(0x3308d751L),
    tole(0xc951d255L), tole(0x806daf72L), tole(0x5b29281bL), tole(0x1215553cL),
    tole(0x230138cfL), tole(0x6a3d45e8L), tole(0xb179c281L), tole(0xf845bfa6L),
    tole(0x021cbaa2L), tole(0x4b20c785L), tole(0x906440ecL), tole(0xd9583dcbL),
    tole(0x613a3c15L), tole(0x28064132L), tole(0xf342c65bL), tole(0xba7ebb7cL),
    tole(0x4027be78L), tole(0x091bc35fL), tole(0xd25f4436L), tole(0x9b633911L),
    tole(0xa777317bL), tole(0xee4b4c5cL), tole(0x350fcb35L), tole(0x7c33b612L),
    tole(0x866ab316L), tole(0xcf56ce31L), tole(0x14124958L), tole(0x5d2e347fL),
    tole(0xe54c35a1L), tole(0xac704886L), tole(0x7734cfefL), tole(0x3e08b2c8L),
    tole(0xc451b7ccL), tole(0x8d6dcaebL), tole(0x56294d82L), tole(0x1f1530a5L)},
    };

    /*
     * There are multiple 16-bit CRC polynomials in common use, but this is
     * *the* standard CRC-32 polynomial, first popularized by Ethernet.
     * x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
     */
    #define CRCPOLY_LE 0xedb88320
    #define CRCPOLY_BE 0x04c11db7

    /*
     * This is the CRC32c polynomial, as outlined by Castagnoli.
     * x^32+x^28+x^27+x^26+x^25+x^23+x^22+x^20+x^19+x^18+x^14+x^13+x^11+x^10+x^9+
     * x^8+x^6+x^0
     */
    #define CRC32C_POLY_LE 0x82F63B78

    /* Try to choose an implementation variant via Kconfig */

    #define CONFIG_CRC32_SLICEBY8

    #ifdef CONFIG_CRC32_SLICEBY8
    # define CRC_LE_BITS 64
    # define CRC_BE_BITS 64
    #endif
    #ifdef CONFIG_CRC32_SLICEBY4
    # define CRC_LE_BITS 32
    # define CRC_BE_BITS 32
    #endif
    #ifdef CONFIG_CRC32_SARWATE
    # define CRC_LE_BITS 8
    # define CRC_BE_BITS 8
    #endif
    #ifdef CONFIG_CRC32_BIT
    # define CRC_LE_BITS 1
    # define CRC_BE_BITS 1
    #endif

    /*
     * How many bits at a time to use.  Valid values are 1, 2, 4, 8, 32 and 64.
     * For less performance-sensitive, use 4 or 8 to save table size.
     * For larger systems choose same as CPU architecture as default.
     * This works well on X86_64, SPARC64 systems. This may require some
     * elaboration after experiments with other architectures.
     */
    #ifndef CRC_LE_BITS
    #  ifdef CONFIG_64BIT
    #  define CRC_LE_BITS 64
    #  else
    #  define CRC_LE_BITS 32
    #  endif
    #endif
    #ifndef CRC_BE_BITS
    #  ifdef CONFIG_64BIT
    #  define CRC_BE_BITS 64
    #  else
    #  define CRC_BE_BITS 32
    #  endif
    #endif
    /*
     * Little-endian CRC computation.  Used with serial bit streams sent
     * lsbit-first.  Be sure to use cpu_to_le32() to append the computed CRC.
     */
    #if CRC_LE_BITS > 64 || CRC_LE_BITS < 1 || CRC_LE_BITS == 16 || \
        CRC_LE_BITS & CRC_LE_BITS-1
    # error "CRC_LE_BITS must be one of {1, 2, 4, 8, 32, 64}"
    #endif

    /*
     * Big-endian CRC computation.  Used with serial bit streams sent
     * msbit-first.  Be sure to use cpu_to_be32() to append the computed CRC.
     */
    #if CRC_BE_BITS > 64 || CRC_BE_BITS < 1 || CRC_BE_BITS == 16 || \
        CRC_BE_BITS & CRC_BE_BITS-1
    # error "CRC_BE_BITS must be one of {1, 2, 4, 8, 32, 64}"
    #endif

    #if CRC_LE_BITS > 8 || CRC_BE_BITS > 8

    /* implements slicing-by-4 or slicing-by-8 algorithm */
    static inline u32
    crc32_body(u32 crc, unsigned char const *buf, u32 len, const u32 (*tab)[256])
    {
    # ifdef __LITTLE_ENDIAN
    #  define DO_CRC(x) crc = t0[(crc ^ (x)) & 255] ^ (crc >> 8)
    #  define DO_CRC4 (t3[(q) & 255] ^ t2[(q >> 8) & 255] ^ \
               t1[(q >> 16) & 255] ^ t0[(q >> 24) & 255])
    #  define DO_CRC8 (t7[(q) & 255] ^ t6[(q >> 8) & 255] ^ \
               t5[(q >> 16) & 255] ^ t4[(q >> 24) & 255])
    # else
    #  define DO_CRC(x) crc = t0[((crc >> 24) ^ (x)) & 255] ^ (crc << 8)
    #  define DO_CRC4 (t0[(q) & 255] ^ t1[(q >> 8) & 255] ^ \
               t2[(q >> 16) & 255] ^ t3[(q >> 24) & 255])
    #  define DO_CRC8 (t4[(q) & 255] ^ t5[(q >> 8) & 255] ^ \
               t6[(q >> 16) & 255] ^ t7[(q >> 24) & 255])
    # endif
        const u32 *b;
        u32    rem_len;
    # ifdef CONFIG_X86
        u32 i;
    # endif
        const u32 *t0=tab[0], *t1=tab[1], *t2=tab[2], *t3=tab[3];
    # if CRC_LE_BITS != 32
        const u32 *t4 = tab[4], *t5 = tab[5], *t6 = tab[6], *t7 = tab[7];
    # endif
        u32 q;

        /* Align it */
        if ( ((long)buf & 3 && len)) {
            do {
                DO_CRC(*buf++);
            } while ((--len) && ((long)buf)&3);
        }

    # if CRC_LE_BITS == 32
        rem_len = len & 3;
        len = len >> 2;
    # else
        rem_len = len & 7;
        len = len >> 3;
    # endif

        b = (const u32 *)buf;
    # ifdef CONFIG_X86
        --b;
        for (i = 0; i < len; i++) {
    # else
        for (--b; len; --len) {
    # endif
            q = crc ^ *++b; /* use pre increment for speed */
    # if CRC_LE_BITS == 32
            crc = DO_CRC4;
    # else
            crc = DO_CRC8;
            q = *++b;
            crc ^= DO_CRC4;
    # endif
        }
        len = rem_len;
        /* And the last few bytes */
        if (len) {
            u8 *p = (u8 *)(b + 1) - 1;
    # ifdef CONFIG_X86
            for (i = 0; i < len; i++)
                DO_CRC(*++p); /* use pre increment for speed */
    # else
            do {
                DO_CRC(*++p); /* use pre increment for speed */
            } while (--len);
    # endif
        }
        return crc;
    #undef DO_CRC
    #undef DO_CRC4
    #undef DO_CRC8
    }
    #endif


    /**
     * crc32_le_generic() - Calculate bitwise little-endian Ethernet AUTODIN II
     *          CRC32/CRC32C
     * @crc: seed value for computation.  ~0 for Ethernet, sometimes 0 for other
     *   uses, or the previous crc32/crc32c value if computing incrementally.
     * @p: pointer to buffer over which CRC32/CRC32C is run
     * @len: length of buffer @p
     * @tab: little-endian Ethernet table
     * @polynomial: CRC32/CRC32c LE polynomial
     */
    static inline u32 crc32_le_generic(u32 crc, unsigned char const *p,
                          u32 len, const u32 (*tab)[256],
                          u32 polynomial)
    {
    #if CRC_LE_BITS == 1
        int i;
        while (len--) {
            crc ^= *p++;
            for (i = 0; i < 8; i++)
                crc = (crc >> 1) ^ ((crc & 1) ? polynomial : 0);
        }
    # elif CRC_LE_BITS == 2
        while (len--) {
            crc ^= *p++;
            crc = (crc >> 2) ^ tab[0][crc & 3];
            crc = (crc >> 2) ^ tab[0][crc & 3];
            crc = (crc >> 2) ^ tab[0][crc & 3];
            crc = (crc >> 2) ^ tab[0][crc & 3];
        }
    # elif CRC_LE_BITS == 4
        while (len--) {
            crc ^= *p++;
            crc = (crc >> 4) ^ tab[0][crc & 15];
            crc = (crc >> 4) ^ tab[0][crc & 15];
        }
    # elif CRC_LE_BITS == 8
        /* aka Sarwate algorithm */
        while (len--) {
            crc ^= *p++;
            crc = (crc >> 8) ^ tab[0][crc & 255];
        }
    # else
        crc = htole32(crc);
        crc = crc32_body(crc, p, len, tab);
        crc = letoh32(crc);
    #endif
        return crc;
    }

    #if CRC_LE_BITS == 1
    u32 crc32_le(u32 crc, unsigned char const *p, u32 len)
    {
        return crc32_le_generic(crc, p, len, NULL, CRCPOLY_LE);
    }
    u32 __crc32c_le(u32 crc, unsigned char const *p, u32 len)
    {
        return crc32_le_generic(crc, p, len, NULL, CRC32C_POLY_LE);
    }
    #else
    u32 crc32_le(u32 crc, unsigned char const *p, u32 len)
    {
        return crc32_le_generic(crc, p, len,
                (const u32 (*)[256])crc32table_le, CRCPOLY_LE);
    }
    u32 __crc32c_le(u32 crc, unsigned char const *p, u32 len)
    {
        return crc32_le_generic(crc, p, len,
                (const u32 (*)[256])crc32ctable_le, CRC32C_POLY_LE);
    }
    #endif

    /*
     * This multiplies the polynomials x and y modulo the given modulus.
     * This follows the "little-endian" CRC convention that the lsbit
     * represents the highest power of x, and the msbit represents x^0.
     */
    static u32 gf2_multiply(u32 x, u32 y, u32 modulus)
    {
        u32 product = x & 1 ? y : 0;
        int i;

        for (i = 0; i < 31; i++) {
            product = (product >> 1) ^ (product & 1 ? modulus : 0);
            x >>= 1;
            product ^= x & 1 ? y : 0;
        }

        return product;
    }

    /**
     * crc32_generic_shift - Append len 0 bytes to crc, in logarithmic time
     * @crc: The original little-endian CRC (i.e. lsbit is x^31 coefficient)
     * @len: The number of bytes. @crc is multiplied by x^(8*@len)
     * @polynomial: The modulus used to reduce the result to 32 bits.
     *
     * It's possible to parallelize CRC computations by computing a CRC
     * over separate ranges of a buffer, then summing them.
     * This shifts the given CRC by 8*len bits (i.e. produces the same effect
     * as appending len bytes of zero to the data), in time proportional
     * to log(len).
     */
    static u32 crc32_generic_shift(u32 crc, u32 len,
                               u32 polynomial)
    {
        u32 power = polynomial; /* CRC of x^32 */
        int i;

        /* Shift up to 32 bits in the simple linear way */
        for (i = 0; i < 8 * (int)(len & 3); i++)
            crc = (crc >> 1) ^ (crc & 1 ? polynomial : 0);

        len >>= 2;
        if (!len)
            return crc;

        for (;;) {
            /* "power" is x^(2^i), modulo the polynomial */
            if (len & 1)
                crc = gf2_multiply(crc, power, polynomial);

            len >>= 1;
            if (!len)
                break;

            /* Square power, advancing to x^(2^(i+1)) */
            power = gf2_multiply(power, power, polynomial);
        }

        return crc;
    }

    u32 crc32_le_shift(u32 crc, u32 len)
    {
        return crc32_generic_shift(crc, len, CRCPOLY_LE);
    }

    u32 __crc32c_le_shift(u32 crc, u32 len)
    {
        return crc32_generic_shift(crc, len, CRC32C_POLY_LE);
    }

    /**
     * crc32_be_generic() - Calculate bitwise big-endian Ethernet AUTODIN II CRC32
     * @crc: seed value for computation.  ~0 for Ethernet, sometimes 0 for
     *  other uses, or the previous crc32 value if computing incrementally.
     * @p: pointer to buffer over which CRC32 is run
     * @len: length of buffer @p
     * @tab: big-endian Ethernet table
     * @polynomial: CRC32 BE polynomial
     */
    static inline u32 crc32_be_generic(u32 crc, unsigned char const *p,
                          u32 len, const u32 (*tab)[256],
                          u32 polynomial)
    {
    #if CRC_BE_BITS == 1
        int i;
        while (len--) {
            crc ^= *p++ << 24;
            for (i = 0; i < 8; i++)
                crc =
                    (crc << 1) ^ ((crc & 0x80000000) ? polynomial :
                          0);
        }
    # elif CRC_BE_BITS == 2
        while (len--) {
            crc ^= *p++ << 24;
            crc = (crc << 2) ^ tab[0][crc >> 30];
            crc = (crc << 2) ^ tab[0][crc >> 30];
            crc = (crc << 2) ^ tab[0][crc >> 30];
            crc = (crc << 2) ^ tab[0][crc >> 30];
        }
    # elif CRC_BE_BITS == 4
        while (len--) {
            crc ^= *p++ << 24;
            crc = (crc << 4) ^ tab[0][crc >> 28];
            crc = (crc << 4) ^ tab[0][crc >> 28];
        }
    # elif CRC_BE_BITS == 8
        while (len--) {
            crc ^= *p++ << 24;
            crc = (crc << 8) ^ tab[0][crc >> 24];
        }
    # else
        crc = htobe32(crc);
        crc = crc32_body(crc, p, len, tab);
        crc = betoh32(crc);
    # endif
        return crc;
    }

    #if CRC_LE_BITS == 1
    u32 crc32_be(u32 crc, unsigned char const *p, u32 len)
    {
        return crc32_be_generic(crc, p, len, NULL, CRCPOLY_BE);
    }
    #else
    u32 crc32_be(u32 crc, unsigned char const *p, u32 len)
    {
        return crc32_be_generic(crc, p, len,
                (const u32 (*)[256])crc32table_be, CRCPOLY_BE);
    }
    #endif
}

