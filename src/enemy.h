// 5x4
const unsigned char enemy_0_0[]={231, 252, 252, 219, 255, 234, 254, 253, 213, 255, 253, 243, 243, 254, 255, 255, 255, 255, 255, 255};
const unsigned char enemy_0_1[]={223, 249, 252, 246, 239, 213, 253, 255, 254, 234, 255, 246, 243, 249, 255, 255, 255, 255, 255, 255};
const unsigned char enemy_0_2[]={223, 243, 243, 239, 255, 234, 251, 247, 213, 255, 246, 207, 207, 249, 255, 255, 255, 255, 255, 255};
const unsigned char enemy_0_3[]={255, 231, 243, 219, 255, 213, 247, 255, 251, 234, 253, 219, 207, 231, 254, 255, 255, 255, 255, 255};
const unsigned char enemy_0_4[]={255, 207, 207, 255, 255, 233, 239, 223, 214, 255, 218, 255, 255, 229, 255, 255, 252, 252, 255, 255};
const unsigned char enemy_0_5[]={255, 223, 207, 239, 255, 215, 222, 255, 237, 235, 245, 239, 255, 223, 250, 255, 253, 252, 254, 255};
};

// 5x4
const unsigned char enemy_1_0[]={219, 226, 210, 206, 255, 196, 237, 222, 226, 255, 247, 244, 244, 253, 255, 255, 255, 255, 255, 255};
const unsigned char enemy_1_1[]={247, 197, 241, 201, 239, 221, 202, 252, 197, 251, 255, 249, 248, 250, 254, 255, 255, 255, 255, 255};
const unsigned char enemy_1_2[]={239, 203, 203, 251, 255, 209, 246, 249, 200, 255, 220, 210, 209, 246, 255, 255, 255, 255, 255, 255};
const unsigned char enemy_1_3[]={223, 215, 199, 231, 255, 247, 232, 243, 212, 238, 253, 228, 227, 232, 251, 255, 255, 255, 255, 255};
const unsigned char enemy_1_4[]={255, 239, 239, 239, 255, 198, 216, 228, 227, 255, 241, 203, 199, 216, 255, 253, 253, 253, 255, 255};
const unsigned char enemy_1_5[]={255, 223, 223, 223, 255, 221, 225, 204, 210, 251, 247, 210, 207, 225, 238, 255, 254, 254, 254, 255};
};

// 5x4
const unsigned char enemy_2_0[]={205, 225, 209, 231, 255, 209, 237, 222, 200, 255, 254, 248, 248, 251, 255, 255, 255, 255, 255, 255};
const unsigned char enemy_2_1[]={223, 198, 242, 202, 251, 247, 202, 252, 197, 238, 253, 245, 244, 246, 255, 255, 255, 255, 255, 255};
const unsigned char enemy_2_2[]={247, 199, 199, 223, 255, 196, 246, 249, 226, 255, 249, 226, 225, 236, 255, 255, 255, 255, 255, 255};
const unsigned char enemy_2_3[]={255, 219, 203, 235, 239, 221, 232, 243, 212, 251, 247, 212, 211, 216, 254, 255, 255, 255, 255, 255};
const unsigned char enemy_2_4[]={223, 223, 223, 255, 255, 211, 216, 228, 201, 255, 228, 203, 199, 242, 255, 255, 254, 254, 254, 255};
const unsigned char enemy_2_5[]={255, 239, 239, 239, 255, 247, 225, 204, 210, 238, 221, 210, 207, 225, 251, 255, 253, 253, 253, 255};
};

const unsigned char enemy_sizex[]={5, 5, 5};
const unsigned char enemy_sizey[]={4, 4, 4};
unsigned char* enemy_ptrs[3][6]={{enemy_0_0, enemy_0_1, enemy_0_2, enemy_0_3, enemy_0_4, enemy_0_5, }, 
{enemy_1_0, enemy_1_1, enemy_1_2, enemy_1_3, enemy_1_4, enemy_1_5, }, 
{enemy_2_0, enemy_2_1, enemy_2_2, enemy_2_3, enemy_2_4, enemy_2_5, },};