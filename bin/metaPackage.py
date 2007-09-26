#!/usr/bin/env python
# -*- coding: iso-8859-15 -*-
# metaPackage.py
#  
#  Author: Laudeci Oliveira <laudeci@gmail.com>
#          Rafael Proença   <cypherbios@ubuntu.com>
# 
#  This program is free software; you can redistribute it and/or 
#  modify it under the terms of the GNU General Public License as 
#  published by the Free Software Foundation; either version 2 of the
#  License, or (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA

#python packages
import apt_inst
import apt_pkg
import config
import datetime
import gobject
import gtk
import gtk.glade
import os
import pango
import pygtk
import shutil
import utils
import msg
import re
#debug only
import  traceback
import sys

packageErrorList = []

grayline = "<span foreground='gray'><b>%s</b>\n<small>%s</small></span>"
greenline = "<span foreground='#b84'><b>%s</b>\n<small>%s</small></span>"
def get_ErrorCount():
	return len(packageErrorList)

def get_ErrorList():
	return packageErrorList

# General procedures
def createNewItem(fileName,isCustom = False,path = ""):
		"""This procedures creates the new truple for ListStore inserts."""

		tmp = packageFile()
		tmp.loadPkg(fileName,path)
		if tmp.get_bad_package():
			packageErrorList.append(tmp)
			il = ([True, grayline % (tmp.get_pkg_Name() , (msg.MESSAGE_0046 % tmp.get_pkg_Version() + ' - ' + msg.MESSAGE_0047 % tmp.get_pkg_SizeText())) ,isCustom,tmp,True,True])
		else:
			il = ([True, "<b>%s</b>\n<small>%s</small>" % (tmp.get_pkg_Name() , (msg.MESSAGE_0046 % tmp.get_pkg_Version() + ' - ' + msg.MESSAGE_0047 % tmp.get_pkg_SizeText())) ,isCustom,tmp,True,False])


		return il

def get_pkg_by_Name(packageList,iterText):
		itemFound = None
		
		e = 0
		d = len(packageList) - 1
		while e <= d:
			m = (e + d) / 2
			if packageList[m][config.C_PKG].get_pkg_Name() == iterText:
				itemFound = packageList[m]
				break
			if packageList[m][config.C_PKG].get_pkg_Name() < iterText: e = m + 1
			else: d = m - 1
		
		return itemFound
	
"""
def get_pkg_by_Name(packageList,iterText):
		itemFound = None
		for iter in packageList:
			tmp = iter[config.C_PKG]
			if tmp.get_pkg_Name() == iterText:
				itemFound = iter
				break
			
		return itemFound
"""
"""		
def itemExistSearch(packageList,iterText,strPath = ""):
		itemFound = False;
		a = packageFile()
		if strPath != "":
			a.loadPkg(os.path.join(strPath,iterText))
		else:
			a.loadPkg(iterText)

		for iter in packageList:
			tmp = iter[config.C_PKG]
			if tmp.get_pkg_FileName() == a.get_pkg_FileName():
				itemFound = True
				break

		return itemFound	
"""
def itemExistSearch(packageList,iterText,strPath = ""):
	itemFound = False;
	a = packageFile()
	if strPath != "":
			a.loadPkg(os.path.join(strPath,iterText))
	else:
			a.loadPkg(iterText)
		
	e = 0
	d = len(packageList) - 1
	while e <= d:
		m = (e + d) / 2
		if packageList[m][config.C_PKG].get_pkg_FileName() == a.get_pkg_FileName():
			itemFound = True
			break
		if packageList[m][config.C_PKG].get_pkg_FileName() < a.get_pkg_FileName(): e = m + 1
		else: d = m - 1
	return itemFound	
#def addNewListItem(packageList,iList, iSelect = False):
#		iter = packageList.append(iList)
#		if iSelect:
#			path = packageList.get_path(iter)
#			col = packageList.parent.get_column(1)
#			packageList.parent.get_selection().select_iter(iter)
#			packageList.parent.scroll_to_cell(path,col)
#			return iter
#		else:
#			return iter
			
def addNewListItem(packageList,iList, iSelect = False, packageView = None):
		iter = packageList.append(iList)
		if iSelect:
			if packageView == None:
				return iter
			path = packageList.get_path(iter)
			col = packageView.get_column(1)
			packageView.get_selection().select_iter(iter)
			packageView.scroll_to_cell(path,col)
			return iter
		else:
			return iter

def isMajorVersion(packageList,item):
		nReturn = True
		nList = packageList

		for n in nList:
			iter = n[config.C_PKG]
			if item.get_pkg_Name() == iter.get_pkg_Name():
				if item.get_pkg_VersionCompare(iter.get_pkg_Version()) < 0:
					nReturn = False
			#elif item.get_pkg_Name() <  iter.get_pkg_Name():
			#	break
		return nReturn
	
def uncheckMinorVersion(packageList):
		tmp = packageList
	
		for e in packageList:
			item = e[config.C_PKG]
			e[config.C_CHECKED] = isMajorVersion(tmp,item)
			if not e[config.C_CHECKED]:
				e[config.C_TITLE]= greenline % (item.get_pkg_Name() , (msg.MESSAGE_0046 % item.get_pkg_Version() + ' - ' + msg.MESSAGE_0047 % item.get_pkg_SizeText()))
			if not e[config.C_DISABLED]:
				e[config.C_CANCHECK] = e[config.C_CHECKED]	

def checkDepends(packageList,dependent):
	tmp = packageList
	newdependents = []
	newdependents.append(dependent)

	for dependent in newdependents:
		for dep in dependent:
			item = get_pkg_by_Name(packageList,dep)
			if item != None and not item[config.C_CHECKED]:
				item[config.C_CHECKED] = isMajorVersion(tmp,item[config.C_PKG])
				if not item[config.C_PKG].depends in newdependents:
					newdependents.append(item[config.C_PKG].depends)
			 
		
def countChecked(packageList):
		
		packagesCount = 0
		packagesSize = 0
	
		customCount = 0
		customSize = 0
		
		selectedCount = 0
		selectedSize = 0
		
		for mFolder in packageList:

			packagesCount += 1
			packagesSize += mFolder[3].get_pkg_Size()
			
			if mFolder[0]:
				selectedCount += 1
				selectedSize += mFolder[3].get_pkg_Size()
			if mFolder[2]:
				customCount += 1
				customSize += mFolder[3].get_pkg_Size()
				
		return [packagesCount, packagesSize, selectedCount, selectedSize , customCount, customSize]					
	
def updatePackageCount(packageList,processit = True):
		
		if processit:
			packCount = countChecked(packageList)
		else:
			packCount = [0,0,0,0,0,0]
		
		msgCount = (" %s / %s ") % (packCount[0], utils.fileSizeFormat(packCount[1]))
		msgCustomCount = (" %s / %s ") % (packCount[4], utils.fileSizeFormat(packCount[5]))
		msgSelected = (" %s / %s ") % (packCount[2], utils.fileSizeFormat(packCount[3]))
		
		return  [msgCount, msgCustomCount, msgSelected,packCount[3]]
	
def removePackage(packageView):
		model, old_iter = packageView.get_selection().get_selected()
		if old_iter:
			model.remove(old_iter)
		return
					
def checkAll(packageList, checkOld = False):
		for iter in packageList:
			if not iter[config.C_DISABLED]:
				if iter[config.C_CANCHECK] or checkOld:
					iter[config.C_CHECKED]	= True
		return
	
def uncheckAll(packageList):
		for iter in packageList:
			if not iter[config.C_DISABLED]:
				if iter[config.C_CANCHECK]:
					iter[config.C_CHECKED]	= False
		return 
	
def invertcheckAll(packageList, checkOld = False):
		for iter in packageList:
			if not iter[config.C_DISABLED]:
				if iter[config.C_CANCHECK] or checkOld:
					iter[config.C_CHECKED] = not iter[config.C_CHECKED]
		return
			
def Column_toggled(path, model, checkOld = False):
		"""Sets the toggled state on the toggle button to true or false."""
		if not model[path][config.C_DISABLED]:
			if model[path][config.C_CANCHECK] or checkOld:
				model[path][config.C_CHECKED] = not model[path][config.C_CHECKED]
		return	
	
class packageFile:
	
	def ___init__(self):
		self._fileName = ""
		self._path = ""
		self._fullFileName = ""
		self._pkgName = ""
		self._pkgSize = ""
		self._pkgSizeText = ""
		self._pkgVersion = ""
		self._pkgDescription = ""
		self._pkgLongDescription = ""
		self._pkgShortDescription = ""
		self._error_loading = False
		self.depends = []
		self.util = utils
		
		apt_pkg.InitConfig()
    	apt_pkg.InitSystem()
		
	def getDependsNames(self,pkgList):	
		ex = re.compile(r"(?si)\'([a-z].*?)\'", re.DOTALL )
		e = []
		st = "%s" % pkgList
		try:
			g= ex.findall(st)
		except:
			exctype, value = sys.exc_info()[:2]
			print self._pkgName,traceback.format_exception_only(exctype, value)

		for n in g:
			e.append(n)
		return e
	
	def loadPkg(self, filename,path = ""):
		try:
			
			if path == "":
				self._fullFileName = os.path.join(config.LOCAL_APT_FOLDER,filename)
				self._path = config.LOCAL_APT_FOLDER
			else:
				self._fullFileName = os.path.join(path,filename)	
				self._path = path
			
			tmppath,tmpfileName= os.path.split(self._fullFileName)
			
			self._fileName = tmpfileName
			
			self._pkgSize = os.path.getsize(self._fullFileName)
			self._pkgSizeText = utils.fileSizeFormat(self._pkgSize)
			
			local = apt_inst.debExtractControl(open(self._fullFileName))
			
			self._sections = apt_pkg.ParseSection(local)
			
			self._pkgName = self._sections["Package"]
			self._pkgVersion = self._sections["Version"]
			self._pkgDescription = self._sections["Description"]
			
			i = self._pkgDescription.find("\n")
			
			self._pkgLongDescription = self._pkgDescription[i+1:]
			self._pkgShortDescription = msg.MESSAGE_0048 + self._pkgName + "\n" + msg.MESSAGE_0049 + self._pkgDescription[:i]
			self.depends=[]
			#get packages where the current packages depends of
			for key in ["Depends","PreDepends"]:
				if self._sections.has_key(key):
					self.depends = self.getDependsNames(apt_pkg.ParseDepends(self._sections[key]))
	
			self._error_loading = False
			return True
			
		except:
			self._fileName = filename
			self._pkgSize = 0
			self._pkgSizeText = utils.fileSizeFormat(self._pkgSize)
			self._pkgName = filename
			self._pkgVersion = '0.0.0'
			self._pkgDescription = msg.MESSAGE_0050
			self._pkgLongDescription = msg.MESSAGE_0050
			self._pkgShortDescription = msg.MESSAGE_0051
			self.depends =[]
			self._error_loading = True
			return False
	
	def get_pkg_depends(self):
		#print self.depends
		return self.depends
	
	def get_pkg_info(self,infoname):
		return self._sections[infoname]
	
	def get_pkg_FileName(self):
		return self._fileName
	
	def get_pkg_Path(self):
		return self._path
	
	def get_pkg_FullFileName(self):
		return self._fullFileName
	
	def get_pkg_Size(self):
		return self._pkgSize
	
	def get_pkg_SizeText(self):
		return self._pkgSizeText
	
	def get_pkg_Name(self):
			return self._pkgName
	
	def get_pkg_Version(self):
		return self._pkgVersion
	
	def get_pkg_Description(self):
		return self._pkgDescription
	
	def get_pkg_LongDescription(self):
		return self._pkgLongDescription
	
	def get_pkg_ShortDescription(self):
		return self._pkgShortDescription 
	
	def get_bad_package(self):
		return self._error_loading 

	def copyTo(self,destination):
		import shutil
		shutil.copyfile(self._fullFileName, destination)

	def get_pkg_VersionCompare(self,version):
		if not self._error_loading:
			return apt_pkg.VersionCompare(self._pkgVersion,version)
		else:
			return -1
	
class MetaPackage(object):
	"""This class handles all requisitions to maintain and create MetaPackage file."""
	#class constructor
	def __init__(self, filename=""):
		self.fileName = filename
		self.mtPackage = 'aptoncd-metapackage'
		self.mtVersion = datetime.date.today().strftime('%Y%m%d')
		self.mtFileName = self.mtPackage + '_' + self.mtVersion + '.deb'
		self.mtSection = 'base'
		self.mtPriority = 'optional'
		self.mtArch = 'i386'
		self.mtMaintainer = 'APTonCD Auto-Packager <http://aptoncd.sourceforge.net>'
		self.mtSDesc = 'auto-meta-package for APTonCD media packages\n'
		self.mtLDesc = '  Auto-generated meta-package that contains as dependencies all\n  packages in APTonCD media, previously generated by APTonCD\n  .\n  To know more about APTonCD Auto-Packager, visit http://aptoncd.sourceforge.net'
		self.mtDesc = self.mtSDesc + self.mtLDesc
		self.packages = ""
		
	def set_mtPackage(self,sValue):
		self.mtPackage = sValue
		
	def set_mtFileName(self,sValue):
		self.mtFileName = sValue
	
	def get_mtFileName(self):
		return self.mtFileName
		
	def set_mtSection(self,sValue):
		self.mtSection = sValue
	
	def set_mtPriority(self,sValue):
		self.mtPriority = sValue
		
	def set_mtArch(self,sValue):
		self.mtArch = sValue
		
	def set_mtMaintainer(self,sValue):
		self.mtMaintainer = sValue
		
	def set_mtSDesc(self,sValue):
		self.mtSDesc = sValue
		
	def set_mtLDesc(self,sValue):
		self.mtLDesc = sValue
		self.mtDesc = self.mtSDesc + self.mtLDes
	
	def appendPackage(self,package):
		self.packages += (package +", ")
	
	def write(self):
		i= 0
		try:
			mFile = open(self.fileName,"a")
			mFile.write("Package: " + self.mtPackage \
			+ "\nVersion: " + self.mtVersion \
			+ "\nSection: " + self.mtSection \
			+ "\nPriority: " + self.mtPriority \
			+ "\nArchitecture: " + self.mtArch \
			+ "\nMaintainer: " + self.mtMaintainer \
			+ "\nDescription: " + self.mtDesc \
			#+ "\nDepends: " + re.sub(',\s$', '\n', self.packages))
			+ "\nDepends: " + self.packages[:len(self.packages)-2] +"\n")
			return True
		except IOError:
			print "The file does not exist, exiting gracefully"
			return False
