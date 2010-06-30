// -*-Mode: C++;-*-

// * BeginRiceCopyright *****************************************************
//
// $HeadURL$
// $Id$
//
// -----------------------------------
// Part of HPCToolkit (hpctoolkit.org)
// -----------------------------------
// 
// Copyright ((c)) 2002-2010, Rice University 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage. 
// 
// ******************************************************* EndRiceCopyright *

//***************************************************************************
//
// File:
//   $HeadURL$
//
// Purpose:
//   [The purpose of this file]
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

//************************* System Include Files ****************************

//*************************** User Include Files ****************************

#include"PathFindMgr.hpp"

//***************************************************************************

//***************************************************************************
// PathFindMgr
//***************************************************************************

static PathFindMgr s_singleton;

PathFindMgr::PathFindMgr()
{
}


PathFindMgr::~PathFindMgr()
{
}


PathFindMgr& 
PathFindMgr::singleton()
{
  return s_singleton;
}


void 
PathFindMgr::addPath(const std::string& path)
{
  std::string fnm = getFileName(path);

  HashMap::iterator it = m_cache.find(fnm);
  if (it == m_cache.end()) {
    std::vector<std::string> pathList;
    pathList.push_back(path);
    m_cache[fnm] = pathList;
  }
  else {
    std::vector<std::string>::iterator it;
    for (it = m_cache[fnm].begin(); it != m_cache[fnm].end(); it++) {
      std::string temp = *it;
      if (temp == path) {
	return;
      }
    }
    m_cache[fnm].push_back(path);
  }
}


bool 
PathFindMgr::getRealPath(std::string& filePath) 
{
  std::string fileName = getFileName(filePath);
  HashMap::iterator it = m_cache.find(fileName);
  
  if (it !=  m_cache.end()) {
    std::vector<std::string> paths = it->second;
    
    if ((filePath[0] == '.' && filePath[1] == '/') || //ambiguous './' path case
	(filePath.find_first_of('/') == filePath.npos)) { //only filename given
      
      filePath = paths[0];
      return true;
    }
    
    std::string toReturn;
    int comparisonDepth = -1;
    std::vector<std::string>::iterator it;

    for (it = paths.begin(); it != paths.end(); it++) {
      std::string currentPath = *it;
      
      if (currentPath == toReturn)
	continue;
      
      size_t currentEIn = currentPath.find_last_of('/');
      //add 1 so that the first '/' will NOT be included
      //won't have to worrry about currentBIn == npos since npos + 1 = 0
      size_t currentBIn = currentPath.find_last_of('/', currentEIn - 1) + 1;
      if (currentBIn > currentEIn)
	currentEIn = currentPath.length();
      size_t sizeA = currentEIn - currentBIn;
      
      
      //these numbers will be the same for every iteration...should consider
      //caching the values for the filePath string.
      size_t fileEIn = filePath.find_last_of('/');
      size_t fileBIn = filePath.find_last_of('/', fileEIn - 1) + 1;
      if (fileBIn > fileEIn)
	fileEIn = filePath.length();
      size_t sizeB = fileEIn - fileBIn;
      
      bool congruent = true;
      int level = -1;
      while (congruent && currentBIn != currentPath.npos &&
	     fileBIn != filePath.npos) { 
	//checks how deep the 2 strings are congruent
	std::string comp1 = currentPath.substr(currentBIn, sizeA);
	std::string comp2 = filePath.substr(fileBIn, sizeB);
	
	if (comp1 == comp2) {
	  level++;
	  currentEIn = currentPath.find_last_of('/', currentBIn - 1);
	  currentBIn = currentPath.find_last_of('/', currentEIn - 1) + 1;
	  sizeA = currentEIn - currentBIn;
	  
	  fileEIn = filePath.find_last_of('/', fileBIn -  1);
	  fileBIn = filePath.find_last_of('/', fileEIn - 1) + 1;
	  sizeB = fileEIn - fileBIn;
	}
	else {
	  congruent = false;
	}
      }
      
      if (level > comparisonDepth) {
	comparisonDepth = level;
	toReturn = currentPath;
      }
    }
    
    if (comparisonDepth == -1) { // if nothing matches beyond the file name
      return false; 
    }
    
    filePath = toReturn;
    return true;
  }
  
  return false; 
}

std::string
PathFindMgr::getFileName(const std::string& path) const
{
  size_t in = path.find_last_of("/");
  if (in != path.npos && path.length() > 1)
    return path.substr(in + 1);
  
  return path;
}