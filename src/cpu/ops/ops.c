#include "gbemu/cpu_ops.h"
#include "ops_private.h"

const CpuOp cpu_ops[256] = {
    op_00, op_01, op_02, op_03, op_04, op_05, op_06, op_07, op_08, op_09, op_0A,
    op_0B, op_0C, op_0D, op_0E, op_0F, op_10, op_11, op_12, op_13, op_14, op_15,
    op_16, op_17, op_18, op_19, op_1A, op_1B, op_1C, op_1D, op_1E, op_1F, op_20,
    op_21, op_22, op_23, op_24, op_25, op_26, op_27, op_28, op_29, op_2A, op_2B,
    op_2C, op_2D, op_2E, op_2F, op_30, op_31, op_32, op_33, op_34, op_35, op_36,
    op_37, op_38, op_39, op_3A, op_3B, op_3C, op_3D, op_3E, op_3F, op_40, op_41,
    op_42, op_43, op_44, op_45, op_46, op_47, op_48, op_49, op_4A, op_4B, op_4C,
    op_4D, op_4E, op_4F, op_50, op_51, op_52, op_53, op_54, op_55, op_56, op_57,
    op_58, op_59, op_5A, op_5B, op_5C, op_5D, op_5E, op_5F, op_60, op_61, op_62,
    op_63, op_64, op_65, op_66, op_67, op_68, op_69, op_6A, op_6B, op_6C, op_6D,
    op_6E, op_6F, op_70, op_71, op_72, op_73, op_74, op_75, op_76, op_77, op_78,
    op_79, op_7A, op_7B, op_7C, op_7D, op_7E, op_7F, op_80, op_81, op_82, op_83,
    op_84, op_85, op_86, op_87, op_88, op_89, op_8A, op_8B, op_8C, op_8D, op_8E,
    op_8F, op_90, op_91, op_92, op_93, op_94, op_95, op_96, op_97, op_98, op_99,
    op_9A, op_9B, op_9C, op_9D, op_9E, op_9F, op_A0, op_A1, op_A2, op_A3, op_A4,
    op_A5, op_A6, op_A7, op_A8, op_A9, op_AA, op_AB, op_AC, op_AD, op_AE, op_AF,
    op_B0, op_B1, op_B2, op_B3, op_B4, op_B5, op_B6, op_B7, op_B8, op_B9, op_BA,
    op_BB, op_BC, op_BD, op_BE, op_BF, op_C0, op_C1, op_C2, op_C3, op_C4, op_C5,
    op_C6, op_C7, op_C8, op_C9, op_CA, op_CB, op_CC, op_CD, op_CE, op_CF, op_D0,
    op_D1, op_D2, op_xx, op_D4, op_D5, op_D6, op_D7, op_D8, op_D9, op_DA, op_xx,
    op_DC, op_xx, op_DE, op_DF, op_E0, op_E1, op_E2, op_xx, op_xx, op_E5, op_E6,
    op_E7, op_E8, op_E9, op_EA, op_xx, op_xx, op_xx, op_EE, op_EF, op_F0, op_F1,
    op_F2, op_F3, op_xx, op_F5, op_F6, op_F7, op_F8, op_F9, op_FA, op_FB, op_xx,
    op_xx, op_FE, op_FF,
};

const CpuOp cpu_cb_ops[64] = {
    cb_00, cb_01, cb_02, cb_03, cb_04, cb_05, cb_06, cb_07, cb_08, cb_09, cb_0A,
    cb_0B, cb_0C, cb_0D, cb_0E, cb_0F, cb_10, cb_11, cb_12, cb_13, cb_14, cb_15,
    cb_16, cb_17, cb_18, cb_19, cb_1A, cb_1B, cb_1C, cb_1D, cb_1E, cb_1F, cb_20,
    cb_21, cb_22, cb_23, cb_24, cb_25, cb_26, cb_27, cb_28, cb_29, cb_2A, cb_2B,
    cb_2C, cb_2D, cb_2E, cb_2F, cb_30, cb_31, cb_32, cb_33, cb_34, cb_35, cb_36,
    cb_37, cb_38, cb_39, cb_3A, cb_3B, cb_3C, cb_3D, cb_3E, cb_3F,
};
