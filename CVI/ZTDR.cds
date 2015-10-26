<distribution version="13.0.2" name="ZTDR" type="MSI">
	<prebuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></prebuild>
	<postbuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></postbuild>
	<msi GUID="{19A16F8B-B0A6-4056-BF6C-E22260352FB7}">
		<general appName="ZTDR" outputLocation="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\Install\cvidistkit.ZTDR_2.0.0" relOutputLocation="..\Install\cvidistkit.ZTDR_2.0.0" outputLocationWithVars="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\Install\cvidistkit.%name_%version" relOutputLocationWithVars="..\Install\cvidistkit.%name_%version" upgradeBehavior="1" autoIncrement="true" version="2.0.0">
			<arp company=" HYPERLABS" companyURL="http://www.hyperlabsinc.com" supportURL="" contact="" phone="" comments=""/>
			<summary title="ZTDR" subject="" keyWords="" comments="" author=""/></general>
		<userinterface language="English" showPaths="true" readMe="" license="">
			<dlgstrings welcomeTitle="Welcome to ZTDR" welcomeText=""/></userinterface>
		<dirs appDirID="102">
			<installDir name="ZTDR" dirID="100" parentID="103" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="HYPERLABS" dirID="101" parentID="104" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="ZTDR" dirID="102" parentID="101" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="HYPERLABS" dirID="103" parentID="7" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="[Start&gt;&gt;Programs]" dirID="7" parentID="-1" isMSIDir="true" visible="true" unlock="false"/>
			<installDir name="C:" dirID="104" parentID="-1" isMSIDir="false" visible="true" unlock="false"/></dirs>
		<files>
			<simpleFile fileID="0" sourcePath="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\CVI\cvibuild.ZTDR\Release\ZTDR.exe" targetDir="102" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="1" sourcePath="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\CVI\FTD2XX.dll" targetDir="102" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/></files>
		<fileGroups>
			<projectOutput targetType="0" dirID="102" projectID="0">
				<fileID>0</fileID></projectOutput>
			<projectDependencies dirID="102" projectID="0">
				<fileID>1</fileID></projectDependencies></fileGroups>
		<shortcuts>
			<shortcut name="ZTDR" targetFileID="0" destDirID="100" cmdLineArgs="" description="" runStyle="NORMAL"/></shortcuts>
		<mergemodules/>
		<products>
			<product name="NI LabWindows/CVI Shared Run-Time Engine" UC="{80D3D303-75B9-4607-9312-E5FC68E5BFD2}" productID="{D3C549FD-7DA5-440B-A1BC-DD92C898949A}" path="(None)" flavorID="_full_" flavorName="Full" verRestr="false" coreVer=""/></products>
		<nonAutoSelectProducts>
			<productID>{D3C549FD-7DA5-440B-A1BC-DD92C898949A}</productID></nonAutoSelectProducts>
		<runtimeEngine installToAppDir="false" activeXsup="false" analysis="false" cvirte="true" dotnetsup="false" instrsup="true" lowlevelsup="false" lvrt="true" netvarsup="false" rtutilsup="false">
			<hasSoftDeps/>
			<doNotAutoSelect>
			<component>activeXsup</component>
			<component>analysis</component>
			<component>cvirte</component>
			<component>dotnetsup</component>
			<component>instrsup</component>
			<component>lowlevelsup</component>
			<component>lvrt</component>
			<component>netvarsup</component>
			<component>rtutilsup</component>
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
		<buildData progressBarRate="3.812563991518227">
			<progressTimes>
				<Begin>0.000000000000000</Begin>
				<ProductsAdded>0.083923547319292</ProductsAdded>
				<DPConfigured>1.269781890446384</DPConfigured>
				<DPMergeModulesAdded>1.780954826340274</DPMergeModulesAdded>
				<DPClosed>2.663980467884252</DPClosed>
				<DistributionsCopied>2.767401127905499</DistributionsCopied>
				<End>26.229067950720037</End></progressTimes></buildData>
	</msi>
</distribution>