#ifndef CALC_FILE_HASHES_TASK_H
#define CALC_FILE_HASHES_TASK_H

#include <QRunnable>
#include "vtfile.h"

class CalcFileHashesTask : public QRunnable
{
  VtFile *file;
public:
  CalcFileHashesTask(VtFile  *file);

  void run(void);
};

#endif // CALC_FILE_HASHES_TASK_H
