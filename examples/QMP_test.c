/*----------------------------------------------------------------------------
 * Copyright (c) 2001      Southeastern Universities Research Association,
 *                         Thomas Jefferson National Accelerator Facility
 *
 * This software was developed under a United States Government license
 * described in the NOTICE file included as part of this distribution.
 *
 * Jefferson Lab HPC Group, 12000 Jefferson Ave., Newport News, VA 23606
 *----------------------------------------------------------------------------
 *
 * Description:
 *      Simple Test Program for QMP
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.4  2002/11/15 15:37:34  chen
 *   Fix bugs caused by rapid creating/deleting channels
 *
 *   Revision 1.3  2002/04/30 18:26:36  chen
 *   Allow QMP_gm to send/recv from itself
 *
 *   Revision 1.2  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.1  2002/04/22 20:28:44  chen
 *   Version 0.95 Release
 *
 *   Revision 1.4  2002/03/27 20:48:50  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.3  2002/02/15 20:34:54  chen
 *   First Beta Release QMP
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <QMP.h>

#define NUM_HANDLES 4

int main (int argc, char** argv)
{
  int i, j;
  QMP_bool_t status, sender, verbose;
  QMP_u32_t  rank;
  QMP_status_t err;
  /*
  QMP_u32_t dims[4] = {2, 2, 4, 2};
  QMP_u32_t ndims = 4;
  */
  QMP_u32_t dims[1] = {2};
  QMP_u32_t ndims = 1;

  void        *rmem[NUM_HANDLES], *smem[NUM_HANDLES];
  QMP_msgmem_t recvmem[NUM_HANDLES];
  QMP_msghandle_t recvh[NUM_HANDLES];
  QMP_msgmem_t sendmem[NUM_HANDLES];
  QMP_msghandle_t sendh[NUM_HANDLES];

  /**
   * Multiple message handles are combined
   */
  QMP_msghandle_t comp_sendh, comp_recvh;

  
  verbose = QMP_FALSE;  
  if (argc > 1 && strcmp (argv[1], "-v") == 0)
    verbose = QMP_TRUE;
  
  QMP_verbose (verbose);
  status = QMP_init_msg_passing (argc, argv, QMP_SMP_ONE_ADDRESS);

  if (status != QMP_SUCCESS) {
    QMP_fprintf(stderr, "QMP_init failed\n");
    return -1;
  }

  status = QMP_declare_logical_topology (dims, ndims);

  if (status == QMP_FALSE)
    QMP_fprintf (stderr, "Cannot declare logical grid\n");
  else
    QMP_fprintf (stderr, "Declare logical grid ok\n");

  rank = QMP_get_node_number ();
  if (rank == 0)
    sender = QMP_TRUE;
  else
    sender = QMP_FALSE;

  for (i = 0; i < NUM_HANDLES; i++) {
    rmem[i] = QMP_allocate_aligned_memory (10234);
    if (!rmem[i]) {
      QMP_fprintf (stderr, "cannot allocate receiving memory\n");
      exit (1);
    }
    recvmem[i] = QMP_declare_msgmem (rmem[i], 10234);
    if (!recvmem[i]) {
      QMP_fprintf (stderr, "recv memory error : %s\n", 
		   QMP_get_error_string(0));
      exit (1);
    }
  }

  for (i = 0; i < NUM_HANDLES; i++) {
    smem[i] = QMP_allocate_aligned_memory (10234);
    if (!smem[i]) {
      QMP_fprintf (stderr, "cannot allocate sending memory\n");
      exit (1);
    }

    sendmem [i]= QMP_declare_msgmem (smem[i], 10234);
    if (!sendmem[i]) {
      QMP_fprintf (stderr, "send memory error : %s\n", 
		   QMP_get_error_string(0));
      exit (1);
    }
  }


  for (i = 0; i < NUM_HANDLES; i++) {
    sendh[i] = QMP_declare_send_relative (sendmem[i], 0, -1, 0);
    if (!sendh[i]) {
      QMP_fprintf (stderr, "Send Handle Error: %s\n", QMP_get_error_string(0));
      exit (1);
    }
  }

  for (i = 0; i < NUM_HANDLES; i++) {
    recvh[i] = QMP_declare_receive_relative (recvmem[i], 0, 1, 0);
    if (!recvh[i]) {
      QMP_fprintf (stderr, "Recv Handle Error: %s\n", 
		   QMP_get_error_string(0));      
      exit (1);
    }
  }

  comp_sendh = QMP_declare_multiple (sendh, NUM_HANDLES);
  comp_recvh = QMP_declare_multiple (recvh, NUM_HANDLES);

  i = 0;
  while (i < 10) {

    if ((err = QMP_start (comp_sendh))!= QMP_SUCCESS)
      QMP_fprintf (stderr, "Start sending failed: %s\n",
		   QMP_error_string(err));
    
    if ((err = QMP_start (comp_recvh)) != QMP_SUCCESS)
      QMP_fprintf (stderr, "Start receiving failed: %s\n", 
		  QMP_error_string(err));
    
    if (QMP_wait (comp_sendh) != QMP_SUCCESS)
      QMP_fprintf (stderr, "Error in sending %d\n", i);
    else
      QMP_fprintf (stderr, "Sending success %d\n", i);

    
    if (QMP_wait (comp_recvh) != QMP_SUCCESS)
      QMP_fprintf (stderr,"Error in receiving %d\n", i);
    else
      QMP_fprintf (stderr, "Received ok %d\n", i);

    sleep (1);
    i++;
  }


  QMP_free_msghandle (comp_recvh);
  QMP_free_msghandle (comp_sendh);

  for (j = 0; j < NUM_HANDLES; j++) {
    QMP_free_msghandle (recvh[j]);
    QMP_free_msgmem (recvmem[j]);

    QMP_free_msghandle (sendh[j]);
    QMP_free_msgmem (sendmem[j]);
    
    QMP_free_aligned_memory (rmem[j]);
    QMP_free_aligned_memory (smem[j]);
  }

  QMP_finalize_msg_passing ();

  return 0;
}


