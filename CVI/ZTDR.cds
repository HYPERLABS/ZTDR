<distribution version="13.0.2" name="ZTDR" type="MSI">
	<prebuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></prebuild>
	<postbuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></postbuild>
	<msi GUID="{19A16F8B-B0A6-4056-BF6C-E22260352FB7}">
		<general appName="ZTDR" outputLocation="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\Install\cvidistkit.ZTDR_2.1.0" relOutputLocation="..\Install\cvidistkit.ZTDR_2.1.0" outputLocationWithVars="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\Install\cvidistkit.%name_%version" relOutputLocationWithVars="..\Install\cvidistkit.%name_%version" upgradeBehavior="1" autoIncrement="true" version="2.1.0">
			<arp company=" HYPERLABS" companyURL="http://www.hyperlabsinc.com" supportURL="" contact="" phone="" comments=""/>
			<summary title="ZTDR" subject="" keyWords="" comments="" author=""/></general>
		<userinterface language="English" showPaths="true" readMe="" license="">
			<dlgstrings welcomeTitle="Welcome to ZTDR" welcomeText=""/></userinterface>
		<dirs appDirID="104">
			<installDir name="C:" dirID="100" parentID="-1" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="ZTDR" dirID="101" parentID="103" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="[Start&gt;&gt;Programs]" dirID="7" parentID="-1" isMSIDir="true" visible="true" unlock="false"/>
			<installDir name="HYPERLABS" dirID="102" parentID="100" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="HYPERLABS" dirID="103" parentID="7" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="ZTDR" dirID="104" parentID="102" isMSIDir="false" visible="true" unlock="false"/></dirs>
		<files>
			<simpleFile fileID="0" sourcePath="d:\Dropbox\Documents\HYPERLABS\GitHub\ZTDR\CVI\cvibuild.ZTDR\Release\ZTDR.exe" targetDir="104" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/></files>
		<fileGroups>
			<projectOutput targetType="0" dirID="104" projectID="0">
				<fileID>0</fileID></projectOutput></fileGroups>
		<shortcuts>
			<shortcut name="ZTDR" targetFileID="0" destDirID="101" cmdLineArgs="" description="" runStyle="NORMAL"/></shortcuts>
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
		<buildData progressBarRate="3.722716360892217">
			<progressTimes>
				<Begin>0.000000000000000</Begin>
				<ProductsAdded>0.054236766002764</ProductsAdded>
				<DPConfigured>0.525579232883410</DPConfigured>
				<DPMergeModulesAdded>0.909439516070884</DPMergeModulesAdded>
				<DPClosed>1.816668341178683</DPClosed>
				<DistributionsCopied>1.921861852082008</DistributionsCopied>
				<End>26.862105598620779</End></progressTimes></buildData>
	</msi>
</distribution>
