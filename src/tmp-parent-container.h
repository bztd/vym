#ifndef TMP_PARENT_CONTAINER_H
#define TMP_PARENT_CONTAINER_H

#include "branch-container-base.h"

class TmpParentContainer : public BranchContainerBase {
  public:
    TmpParentContainer ();
    virtual void init();

    virtual void addToBranchesContainer(Container *c);
    virtual void addToImagesContainer(Container *c);

  protected:
    virtual void updateBranchesContainerLayout();

  public:
    virtual void reposition();

};

#endif
