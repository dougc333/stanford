                        3Com (R) Corporation
                     EtherDisk (R) Diskette for
                  The EtherLink 10 ISA NIC Family
          Release Notes and Frequently Asked Questions (FAQ)

This file contains the release notes and answers to some frequently asked
questions about the 3C509B to help you get the most out of your EtherLink
10 ISA Network Interface Card (NIC). This information is updated regularly 
on 3com's world wide web site (www.3com.com).

Release Notes
-------------

EtherDisk 6.1 has been revised and is split into two diskettes.

EtherDisk #1 contains all the files necessary to install the EtherLink 10
ISA in Windows 95/98 or Windows NT.  This includes a Windows based GUI
installer for Windows 95/98 and a post installation configuration and
diagnostic tool, accessible from a system tray icon and from the Windows
Start Menu, for both Windows 9x and Windows NT environments. Both NDIS3 and 
NDIS4 miniport drivers are included and appropriately installed in Windows 95/98 
and Windows NT 3.51 and 4.0.

EtherDisk #2 contains the software that was on EtherDisk 6.0.  This includes
installation for NetWare versions 3.X and 4.X, various DOS based network
operating systems, IBM OS/2, and the packet driver.

Full duplex support for the 3C509B has been added to the NetWare server and
Windows NT/95/98 drivers. Control of the full duplex feature is provided in
the configuration and diagnostic program.  Full duplex operation improves
overall network throughput on twisted pair (TP) networks.

A boot ROM for the EtherLink 10 ISA product family is available from your
network supplier.  The 3C5-TriROM supports NetWare boot, RPL, and BootP
network boot protocols.

The EtherLink 10 ISA, formerly known as the EtherLink III, has gone thru a
product name change.  The products are fully compatible and the terms EtherLink
III and EtherLink 10 ISA can be used interchangeably.

Important
---------

Not all NICS and hardware product features are supported with this EtherDisk.
The 3C509 and 3C509B in both classic and Plug n Play modes are fully supported.
The 3C509 and 3C509B in EISA mode are not supported.
The 3C579 EISA NICs are not supported.
The 3C529 MicroChannel NICs are partially supported.  In Windows 95 and
Windows NT, the NIC driver can be installed and removed, but the windows
diagnostic program and tray icon are not loaded.  To make configuration
changes or test a 3C529, use the DOS diagnotic on diskette #2.

The Windows 95/98 and Windows NT full port drivers have been combined into a
single miniport driver.  It is important to use this driver if you use
3Com's DRMON network management software.  This driver will give better
performance when the DRMON client agent is used.  When converting from past
drivers to this driver, or installing this driver for the first time,
several things must be considered.


Windows NT
----------
Older drivers may be updated directly to the miniport driver.  However, if
this driver will be used in a multiprocessor Windows NT 4.0 system, then
you must update to service pack 3 or newer.  Failure to update to service
pack 3 may cause your system to crash.

If you are installing more than one EtherLink 10 ISA in a Windows NT 4.0 system,
you need to follow the instructions in the manual or the Windows NT
installation notes (WINNT.TXT).  Failure to do so could result in having to
reinstall Windows NT 4.0.

This version of the EtherDisk does not contain driver support for Windows 2000.
In order to install the 3C509B in the Windows 2000 environment use the driver
supplied by Microsoft that is native to Windows 2000.


Windows 95/98
-------------
Install Windows 95/98 before installing the EtherLink 10 ISA NIC.  After
installing Windows 95/98, run PREINSTL on EtherDisk #2 and follow the
instructions.

Older, full port drivers (ELNK3.VXD) cannot be updated directly to the
miniport driver due to the driver name change.  To update your older
driver you must follow the update procedure in W95NDIS3.TXT on EtherDisk #2.
For users upgrading from EtherDisk v6.0x driver set please refer to W95NDIS3.TXT
for specific upgrade instructions.


Windows For Workgroups (WFW)
----------------------------
For Windows for Workgroups requiring the standard NDIS3, 32-bit protected mode
diver (ELNK3.386), support refer to the file WFWNDIS3.TXT in the HELP directory 
on EtherDisk #2.

For users requiring real-mode, NDIS2 support (ELNK3.DOS) for Windows for 
Workgroups refer to the HELP file WFWNDIS2.TXT for installation instructions.

Users requiring DOS ODI support (3C5X9.COM) in a Windows for Workgroups 
environment refer to the HELP file WFWNETWR.TXT for installation instructions.


Frequently Asked Questions
--------------------------

Q: I installed an EtherLink 10 ISA (3C509B) NIC in my computer, but
   neither the diagnostic and configuration program nor the driver can find 
   the NIC.  What's wrong?

A: Your computer's BIOS is issuing a series of I/O instructions that causes
   the 3C509B to think it's going to be activated as a Plug 'N Play (PnP)
   device.   Unfortunately, the NIC waits for the PnP series to complete and
   ignores the "classic" or "legacy" method for discovering an EtherLink 10 ISA
   NIC.  The fix for this problem is very simple. Follow the steps below:

 1 - Boot a minimal DOS setup, making sure that no EtherLink 10 ISA drivers
     are loaded.
 2 - Put this EtherDisk in the diskette drive and type A: at the DOS prompt.
 3 - Enter PNPDSABL at the DOS prompt.  The configuration and diagnostic
     program will execute twice.  The first time it executes, the
     configuration and diagnostic program "kicks" the EtherLink 10 ISA out
     of its PnP wait state.  During the second execution, it disables
     Plug 'n Play. The final message displayed will be:
        "The 3C5X9 adapter, adapter number 1, was successfully configured"
 4 - Finally, remove the EtherDisk from the diskette drive, and turn the
     computer power off, then on.

   It is also possible that your computer contains a device that uses I/O
   address 0x110.  If possible, move the device at I/O address 0x110 to some
   other non-conflicting address.  All 3Com drivers and the diagnostic software
   use I/O address 0x110 as the default.  If the conflicting device cannot be
   moved away from address 0x110, then the driver (or diagnostic) can be made
   to use another I/O address for discovery.  Look at the documentation for
   each driver for details.

Q: Why does a red X occur on the 3Com Icon in the system tray in the lower
   right hand corner of the screen?

A: The red X indicates that the NIC is not connected to network.  For Twisted
   pair installations that means the NIC is not detecting linkbeat.  If this
   occurs check the cabling and make sure the NIC is set to use twisted pair.

   This may also occur if the NIC is configured for legacy mode and the Input/Output
   Range under the Resource tab in the properties of the EtherLink 10 ISA do not
   match the actual I/O Base Address of the NIC.  To check the NICs I/O Address
   run the diagnostic/configuration utility program.

Q: I noticed that my system hangs when performing the GUI diagnostics when
   the 3C509B is in legacy mode, but they work when the NIC is configured
   for Plug 'n' Play.  What is the problem?

A: This can occur if the resources are not configured properly in the device
   manager for the 3C509 when configured for legacy mode.  Check the IO Base
   Address under the Configuration tab for the GUI diagnostics for the 3C509 
   and make sure they match the settings under the Resources tab in properties
   for the 3C509 in the device manager.
 
Q: My system does not recognize the 3COM 3C509B NIC. What do I do?

A: If your computer has a PnP BIOS, and it doesn't find the 3C509B NIC,
   either the NIC has the PnP feature disabled or the BIOS is unable to
   find the NIC due to the problem described in the previous FAQ.  Boot the
   computer to a minimum DOS configuration.  Put this EtherDisk in the diskette
   driver and type A: at the DOS prompt.  Enter 3C5X9CFG at the next DOS prompt.
   If the diagnostic program emits an error saying that no EtherLink 10 ISA
   adapters were found, follow the instructions in the answer to the previous
   FAQ.  If the diagnostic finds the NIC, enter the configuration menu and
   examine the state of the Plug 'n Play function.  If PnP is set to disabled,
   change it to enabled, and save the configuration by clicking the OK button.

Q: When running the Windows GUI diagnostic I receive a warning that states:
   "By running the diagnostics at this time unpredictable results may occurs".
   What is the problem?

A: This warning generally appears as the result of 2 conditions:

   1) Check to make sure the TCAATDI diagnostic (TCAITDI diagnostic for Windows NT
      users) is installed.  Check in the Network Control Panel Applet to make sure 
      the TCAATDI diagnostic appears under installed components.  For Windows NT 
      users, this can be observed under the Services tab.  If this service is not
      installed then re-install the NIC drivers.

   2) The NIC driver has not been installed properly.  Check in the Device
      Manager and make sure the 3Com EtherLink 10 ISA does not have a yellow !
      or a red x.

Q: I have installed a 3Com EtherLink 10/100 PCI (3C90x) with the 3C509B and 
   now I am having problems with the Windows diagnostic.  What is wrong?

A: This version of the EtherDisk (v6.1) for the EtherLink 10 ISA (3C509B) is
   compatible with the Windows GUI diagnostic from the EtherDisk v4.0x for PCI.
   If later drivers are installed for the EtherLink 10/100 PCI problems will
   occur when running the diagnostic.  In this installation it is recommended
   not to use the diagnostic.  This is due to a change in the diag architecture
   implemented with the PCI product that is not supported with the 3C509B.
   If you need to run the diagnostic use the DOS utility (3C5X9CFG.EXE) on
   EtherDisk #2.

Q: When I load the DOS drivers from EtherDisk 6.1 for the 3C579 (EISA) or 
   3C529 (MCA) the driver bannner shows the 3C509(B), does this driver set support 
   non-ISA products of the 3C5X9 product family?

A: While these drivers will detect non-ISA NICs of the 3C5X9 family, this driver set
   was targeted for the ISA product and was not certified on non-ISA products.  For
   the most recent certified drivers for the non-ISA 3C5X9 product family members
   refer to the 3Com web site (www.3com.com).

Q: Can a 3C509B NIC be installed in an 8 bit slot?

A: Yes.  The 3C509B NIC can be installed in any 8-bit slot.  However, the
   3c509B will only work in a computer with a 286 or higher processor.

Q: What do I need to do to exclude within EMM386 for the 3C5X9 NIC?

A: EMM386 does not need an exclusion for the 3C509 NIC because the card does
   not use shared memory.

Q: I installed the 3C509 in an ISA slot but I get an error that states the
   NIC is configured for EISA mode.  How do I fix this problem?

A: Insert the 3C509 NIC in an EISA slot and use the 3Com configuration
   utility to reconfigure the NIC's I/O address mode from EISA to non-EISA.
   Save the setting for the NIC and exit.  This will return the NIC to the
   default mode, which is ISA.

Q: Where do I get the latest drivers?

A: The latest 3Com drivers can be found on any of these three online
   locations:
      3Com's CompuServe forum (go NetForum)
      3Com bulletin board service (BBS)
      3Coms World Wide Web site (http://www.3Com.com).

Q: My system has PCI slots and ISA slots. When I install the NIC, I get an
   error stating there is an interrupt conflict or, the system crashes when the
   NIC driver loads or the diagnostic executes the interrupt test.  How do I
   fix this problem?

A: Make sure that the PCI configuration, usually found in the BIOS setup of
   the computer, is not allocating the interrupt request level (IRQ) for use by
   the PCI bus.  You must either not allow the IRQ to be used for a PCI slot
   or move the IRQ for the 3C509 ISA NIC to a level that won't conflict.

Q: I am getting a conflict message when loading a packet driver. How do I
   fix this problem?

A: When loading the packet driver, the software interrupt may need to be
   changed to avoid conflict. Packet drivers information is found in the
   \HELP\PACKET.TXT file on EtherDisk #2.

Q: I am using VLMs for NetWare and Windows/ WFW.  How do I configure my
   Windows environment to support VLMs?

A: If your NetWare shell version within Windows doesn't support VLMs, you
   will need to download the NetWare Client Kit for Windows/ DOS from Novell's
   Netwire or run the Intelligent Auto Installation utility on EtherDisk #2.
   This utility will update your Window's support for NetWare by installing
   the latest windows files.

Q: I am using Microsoft drivers and having trouble connecting the NIC in WFW.

A: Use the 3Com drivers instead of Microsoft drivers. There are help files
   to assist with these installations, and the install program allows updating
   these drivers.

Q: Can I do a Windows Custom install with the 3C509 using the drivers from
   EtherDisk 6.1?

A: For users requiring a Windows custom install refer to the W95NDIS3.TXT file
   on EtherDisk #2 in the HELP directory for details.

Q: My NetWare workstation will not connect to the file server. How can I fix
   this problem?

A: The most common problem is the client and server are using different
   NetWare frame types.  The default frame type for NetWare 4.x and 3.12 is
   802.2, where NetWare 3.11 used 802.3 as the default frame type.  To verify
   this problem,
 1 - Verify the frame type of the file server.  This information is in the
     AUTOEXEC.NCF file on the server. Check to see which frame types are
     supported by the server.
 2 - Edit the NET.CFG file to set the proper frame type.  All the valid
     frame types are shown below:

     Link Driver 3C5X9
         Frame Ethernet_802.2
         Frame Ethernet_802.3
         Frame Ethernet_II
         Frame Ethernet_Snap

Q: I have Award BIOS 4.50PG and I can not get into the 3Com configuration
   program.

A: With the Award 4.50pg BIOS, you could either update the BIOS to a more
   current version or execute PNPDSABL.BAT to disable the PnP of the NIC.

Q: The "NIC Details..." option under the General tab of the GUI diagnostic
   for the 3C5009 does not always show the correct transceiver type, when
   AUTO media selection is determined.  What is the problem?

A: The transceiver type initially shows the EEPROM setting which is not
   always accurate.  The transceiver setting will be updated after running
   the diagnostics, and will reflect the accurate value.

Q: Are there any command line functions or options for the 3Com diagnostics
   program?

A: Yes.  The INSTRUCT.TXT file located in the \HELP sub-directory on
   EtherDisk #2 explains in detail the possible options and functions of the
   diagnostics program.

Q: The 3Com Tray Icon does not appear on my computer can I run the Windows
   GUI diagnostic or enable it so it does appear in the system tray?

A: The GUI diagnostic and Help can be executed from the Windows Start Menu.  
   It is accessible from Start -> Programs -> 3Com NIC Utilities -> 3COM
   NIC DOCTOR.  To enable the tray icon execute the 3Com NIC Doctor as
   explained earlier, and select the "Show Icon in System Tray" under the
   General tab.

Q: Can I remove the 3Com NIC Doctor from the System tray in Windows?

A: Yes, this can be done by executing the 3Com NIC Doctor GUI diagnostic as
   explained in the previous FAQ and deselect the "Show Icon in System Tray" 
   under the General tab.

Q: When I click on the "Show Icon in System Tray" check box to deselect it, an
   "This program has performed an illegal operation...", what happened?

A: This problem occurs intermittently and from testing does not have a system 
   impact.  Ignore this error, and follow the instructions included in this 
   file if you wish to enable the tray icon.  

Q: Are there MSL drivers for the 3Com 3C5X9 NIC family?

A: No, the 3C5X9 NIC cannot be used as a MSL with Novell.

Q: When configuring multiple NICs with Windows GUI diagnostic, the new settings
   do not always appear under the Configuration tab, what is the problem?

A: The GUI diagnostic may not show the current configuration changes in a
   multiple NIC configuration.  They will appear upon restart of the system.

Q: Where can I get additional tech tips?

A: To keep up with support of newer platforms, look to the 3Com Web site
   or the 3Com Fax Back system (408-727-7021) for additional tech tips.
   Request document 9999 for a list of documents available.

%VER - 3C5X9 Release Notes and FAQ V6.1d

