<distribution version="13.0.2" name="ZTDR" type="MSI">
	<prebuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></prebuild>
	<postbuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></postbuild>
	<msi GUID="{19A16F8B-B0A6-4056-BF6C-E22260352FB7}">
		<general appName="ZTDR" outputLocation="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\Install\cvidistkit.ZTDR_2.0.4" relOutputLocation="..\Install\cvidistkit.ZTDR_2.0.4" outputLocationWithVars="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\Install\cvidistkit.%name_%version" relOutputLocationWithVars="..\Install\cvidistkit.%name_%version" upgradeBehavior="1" autoIncrement="true" version="2.0.4">
			<arp company=" HYPERLABS" companyURL="http://www.hyperlabsinc.com" supportURL="" contact="" phone="" comments=""/>
			<summary title="ZTDR" subject="" keyWords="" comments="" author=""/></general>
		<userinterface language="English" showPaths="true" readMe="" license="">
			<dlgstrings welcomeTitle="Welcome to ZTDR" welcomeText=""/></userinterface>
		<dirs appDirID="102">
			<installDir name="ZTDR" dirID="100" parentID="104" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="HYPERLABS" dirID="101" parentID="103" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="ZTDR" dirID="102" parentID="101" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="C:" dirID="103" parentID="-1" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="[Start&gt;&gt;Programs]" dirID="7" parentID="-1" isMSIDir="true" visible="true" unlock="false"/>
			<installDir name="HYPERLABS" dirID="104" parentID="7" isMSIDir="false" visible="true" unlock="false"/></dirs>
		<files>
			<simpleFile fileID="0" sourcePath="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\CVI\FTD2XX.dll" targetDir="102" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="1" sourcePath="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\CVI\cvibuild.ZTDR\Release\ZTDR.exe" targetDir="102" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/></files>
		<fileGroups>
			<projectOutput targetType="0" dirID="102" projectID="0">
				<fileID>1</fileID></projectOutput>
			<projectDependencies dirID="102" projectID="0">
				<fileID>0</fileID></projectDependencies></fileGroups>
		<shortcuts>
			<shortcut name="ZTDR" targetFileID="1" destDirID="100" cmdLineArgs="" description="" runStyle="NORMAL"/></shortcuts>
		<mergemodules/>
		<products>
			<product name="NI LabWindows/CVI Shared Run-Time Engine" UC="{80D3D303-75B9-4607-9312-E5FC68E5BFD2}" productID="{D3C549FD-7DA5-440B-A1BC-DD92C898949A}" path="C:\ProgramData\National Instruments\MDF\ProductCache\" flavorID="_full_" flavorName="Full" verRestr="false" coreVer="">
				<cutSoftDeps/></product></products>
		<nonAutoSelectProducts>
			<productID>{D3C549FD-7DA5-440B-A1BC-DD92C898949A}</productID></nonAutoSelectProducts>
		<runtimeEngine installToAppDir="false" activeXsup="true" analysis="true" cvirte="true" dotnetsup="true" instrsup="true" lowlevelsup="true" lvrt="true" netvarsup="true" rtutilsup="true">
			<hasSoftDeps/>
			<doNotAutoSelect>
			<component>instrsup</component>
			<component>lvrt</component>
			</doNotAutoSelect></runtimeEngine><sxsRuntimeEngine>
			<selected>false</selected>
			<doNotAutoSelect>false</doNotAutoSelect></sxsRuntimeEngine>
		<advanced mediaSize="650">
			<launchConditions>
				<condition>MINOS_WINXP</condition>
			</launchConditions>
			<includeConfigProducts>true</includeConfigProducts>
			<maxImportVisible>silent</maxImportVisible>
			<maxImportMode>merge</maxImportMode>
			<custMsgFlag>false</custMsgFlag>
			<custMsgPath>msgrte.txt</custMsgPath>
			<signExe>true</signExe>
			<certificate></certificate>
			<signTimeURL></signTimeURL>
			<signDescURL></signDescURL></advanced>
		<Projects NumProjects="1">
			<Project000 ProjectID="0" ProjectAbsolutePath="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\CVI\ZTDR.prj" ProjectRelativePath="ZTDR.prj"/></Projects>
		<buildData progressBarRate="4.823015702447026">
			<progressTimes>
				<Begin>0.000000000000000</Begin>
				<ProductsAdded>0.059196193414912</ProductsAdded>
				<DPConfigured>0.738685236305798</DPConfigured>
				<DPMergeModulesAdded>1.302575228292534</DPMergeModulesAdded>
				<DPClosed>2.119203683485532</DPClosed>
				<DistributionsCopied>2.225990265988187</DistributionsCopied>
				<End>20.733915493840001</End></progressTimes></buildData>
	</msi>
</distribution>
