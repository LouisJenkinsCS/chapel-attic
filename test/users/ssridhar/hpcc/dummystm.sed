# This script converts the 
s/CHPL_STM_LOAD/CHPL_DUMMY_STM_LOAD/g
s/CHPL_STM_ARRAY_LOAD/CHPL_DUMMY_STM_ARRAY_LOAD/g
s/CHPL_STM_TX_ALLOC_PERMIT_ZERO/CHPL_DUMMY_STM_TX_ALLOC_PERMIT_ZERO/g
s/chpl_stm_tx_free(tx, /chpl_free(/g
