#pragma once

#include "library/math.h"

struct mstudiobbox_t {
  int       bone;
  int       group; // intersection group
  vector3_t bbmin; // bounding box
  vector3_t bbmax;
  int       szhitboxnameindex; // offset to the name of the hitbox.
  int       unused[8];

  const char* pszHitboxName() {
    if (szhitboxnameindex == 0)
      return "";

    return ((const char*)this) + szhitboxnameindex;
  }

  mstudiobbox_t() {}

private:
  // No copy constructors allowed
  mstudiobbox_t(const mstudiobbox_t& vOther);
};

struct mstudiohitboxset_t {
  int                   sznameindex;
  inline char* const    pszName(void) const { return ((char*)this) + sznameindex; }
  int                   numhitboxes;
  int                   hitboxindex;
  inline mstudiobbox_t* pHitbox(int i) const {
    return (mstudiobbox_t*)(((std::byte*)this) + hitboxindex) + i;
  };
};

struct mstudiobone_t {
  int                sznameindex;
  inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
  int                parent;            // parent bone
  int                bonecontroller[6]; // bone controller index, -1 == none

  // default values
  vector3_t pos;
  float     quat[4];
  vector3_t rot;
  // compression scale
  vector3_t posscale;
  vector3_t rotscale;

  float        poseToBone[3][4];
  float        qAlignment[4];
  int          flags;
  int          proctype;
  int          procindex;   // procedural rule
  mutable int  physicsbone; // index into physically simulated bone
  inline void* pProcedure() const {
    if (procindex == 0)
      return NULL;
    else
      return (void*)(((std::byte*)this) + procindex);
  };
  int                surfacepropidx; // index into string tablefor property name
  inline char* const pszSurfaceProp(void) const { return ((char*)this) + surfacepropidx; }
  int                contents; // See BSPFlags.h for the contents flags

  int unused[8]; // remove as appropriate

  mstudiobone_t() {}

private:
  // No copy constructors allowed
  mstudiobone_t(const mstudiobone_t& vOther);
};

struct studio_header_t {
  int id;
  int version;

  int checksum;

  char name[64];
  int  length;

  vector3_t eyeposition;

  vector3_t illumposition;

  vector3_t hull_min;
  vector3_t hull_max;

  vector3_t view_bbmin;
  vector3_t view_bbmax;

  int flags;

  int numbones;
  int boneindex;

  inline mstudiobone_t* GetBone(int i) const {
    return (mstudiobone_t*)(((std::byte*)this) + boneindex) + i;
  };
  //	inline mstudiobone_t *pBone(int i) const { Assert(i >= 0 && i < numbones); return
  //(mstudiobone_t *)(((byte
  //*)this) + boneindex) + i; };

  int numbonecontrollers;
  int bonecontrollerindex;

  int numhitboxsets;
  int hitboxsetindex;

  mstudiohitboxset_t* GetHitboxSet(int i) const {
    return (mstudiohitboxset_t*)(((std::byte*)this) + hitboxsetindex) + i;
  }

  inline mstudiobbox_t* GetHitbox(int i, int set) const {
    mstudiohitboxset_t const* s = GetHitboxSet(set);

    if (!s)
      return NULL;

    return s->pHitbox(i);
  }

  inline int GetHitboxCount(int set) const {
    mstudiohitboxset_t const* s = GetHitboxSet(set);

    if (!s)
      return 0;

    return s->numhitboxes;
  }

  int numlocalanim;
  int localanimindex;

  int numlocalseq;
  int localseqindex;

  mutable int activitylistversion;
  mutable int eventsindexed;

  int numtextures;
  int textureindex;

  int numcdtextures;
  int cdtextureindex;

  int numskinref;
  int numskinfamilies;
  int skinindex;

  int numbodyparts;
  int bodypartindex;

  int numlocalattachments;
  int localattachmentindex;

  int numlocalnodes;
  int localnodeindex;
  int localnodenameindex;

  int numflexdesc;
  int flexdescindex;

  int numflexcontrollers;
  int flexcontrollerindex;

  int numflexrules;
  int flexruleindex;

  int numikchains;
  int ikchainindex;

  int nummouths;
  int mouthindex;

  int numlocalposeparameters;
  int localposeparamindex;

  int surfacepropindex;

  int keyvalueindex;
  int keyvaluesize;

  int numlocalikautoplaylocks;
  int localikautoplaylockindex;

  float mass;
  int   contents;

  int numincludemodels;
  int includemodelindex;

  mutable void* virtualModel;

  int szanimblocknameindex;
  int numanimblocks;
  int animblockindex;

  mutable void* animblockModel;

  int bonetablebynameindex;

  void* pVertexBase;
  void* pIndexBase;

  std::byte constdirectionallightdot;

  std::byte rootLOD;

  std::byte numAllowedRootLODs;

  std::byte unused[1];

  int unused4;

  int   numflexcontrollerui;
  int   flexcontrolleruiindex;
  float flVertAnimFixedPointScale;
  int   unused3[1];
  int   studiohdr2index;
  int   unused2[1];
};
