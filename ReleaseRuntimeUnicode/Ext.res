        ��  ��                  �      �� ��     0 	        �4   V S _ V E R S I O N _ I N F O     ���                                        :   S t r i n g F i l e I n f o      0 4 0 9 0 4 e 4   8   C o m p a n y N a m e     C o n c e p t G a m e   N   F i l e D e s c r i p t i o n     O p e n C a p t u r e   O b j e c t     *   F i l e V e r s i o n     1 . 2 1     2 	  I n t e r n a l N a m e   O p e n C a p t     ^   L e g a l C o p y r i g h t   C o p y r i g h t   �   2 0 1 4   C o n c e p t G a m e     H   O r i g i n a l F i l e n a m e   O p e n C a p t u r e . m f x   :   P r o d u c t V e r s i o n   1 ,   0 ,   0 ,   1     2 	  S p e c i a l B u i l d   0 0 0 0 0 0 0 1     D    V a r F i l e I n f o     $    T r a n s l a t i o n     	�       ��
 ���     0 	        %Y�#  (   E D I F   ��f       0 	        {
    "About": {
        "Name": "Open Capture v1.21",
        "Author": "ConceptGame",
        "Copyright": "Copyright (C) 2014 ConceptGame",
        "Comment": "Process video capture images",
        "URL": "none",
        "Help": "Help/OpenCapture.chm",
        "Identifier": "OCAP"
    },
    "ActionMenu": [
		"Separator",
		["Capture/Preview",
            [0,"Choose Device"],
			[1,"Start Capture"],
			[2,"Stop Capture"],
			[3,"Start Preview"],
			[4,"Stop Preview"]
		],
		["Properties",
            [18,"Mirror Image"],
			[29,"Set orientation"],
			[19,"Set resolution"],
			[20, "Set size"],
			[22, "Set camera property"]
		],
		["Track Color",
            [5,"Set Tracking Color range in HSV"],
			[6,"Start tracking color"],
			[7,"Stop tracking Color"]
		],
		["Save to file",
            [8,"Save current frame to file"],
			[21,"Save raw frame at resolution to file"],
			[9,"Start saving video stream to file"],
			[10,"Stop saving video stream to file"]
		],
		["Miscellaneous",
            [11,"Set Background Color"]
		],
		["Track Object",
            [12,"Load object classifier"],
			[13,"Start tracking object"],
			[14,"Stop tracking object"]
		],
		["Track Feature",
            [15,"Load feature object"],
			[16,"Start tracking feature"],
			[17,"Stop tracking feature"]
		],
		["Track Barcode",
            [23,"Start tracking barcode"],
			[24,"Stop tracking barcode"],
			[25,"Enable auto detection"],
			[26,"Disable auto detection"],
			[27,"Enable sharpening"],
			[28,"Disable sharpening"]
		],
		"Separator"  
    ],
	"ConditionMenu": [
		"Separator",
        [0,"Is device initializing?"],
		[1,"Is device running?"],
		[2,"Has an error?"],
		"Separator",
		["Track Barcode",
			[3,"Barcode Found?"]
		],
		"Separator"
    ],
    "ExpressionMenu": [
		"Separator",
		[0,"Last Error"],
		["Track Color",
			[1,"Get Color Position X from Id"],
			[2,"Get Color Position Y from Id"],
			[3,"Get HSV color from positions"],
			[4,"Get RGB color from positions"],
			[5,"Get Track Color Object Width"],
			[6,"Get Track Color Object Height"]
		],
		["Track Object",
			[7,"Get Object Position X"],
			[8,"Get Object Position Y"],
			[9,"Get Object Width"],
			[10,"Get Object Height"]
		],
		["Track Feature",
			[11,"Get number of good matches"],
			[12,"Get Corner Position X of Feature Object"],
			[13,"Get Corner Position X of Feature Object"]
		],
		["Track Barcode",
			[14,"Barcode Text"]
		],
		"Separator"
	],
    "Actions": [
        {
            "Title": "Choose Device %0",
            "Parameters": [
                ["Integer","Device Id (based on 0)"]
            ]
        },
		{
            "Title": "Start Capture"
        },
		{
            "Title": "Stop Capture"
        },
		{
            "Title": "Start Preview"
        },
		{
            "Title": "Stop Preview"
        },
		{
            "Title": "Set Tracking Color%0 range from HSVlow=(%1,%2,%3) to HSVhigh=(%4,%5,%6)",
            "Parameters": [
                ["Integer","Color Id (1 or 2)"],
				["Integer","Hue low value [0-179]"],
				["Integer","Saturation low value [0-255]"],
				["Integer","Value low value [0-255]"],
				["Integer","Hue high value [0-179]"],
				["Integer","Saturation high value [0-255]"],
				["Integer","Value high value [0-255]"]
            ]
        },
		{
            "Title": "Start tracking color%0 ",
            "Parameters": [
                ["Integer","Tracking Color Id (1=first, 2=second, 3=both)"]
			]
        },
		{
            "Title": "Stop tracking color%0 ",
            "Parameters": [
                ["Integer","Tracking Color Id (1=first, 2=second, 3=both)"]
			]
        },
		{
            "Title": "Save current frame to %0 ",
            "Parameters": [
                ["Text","Filename Path"]
			]
        },
		{
            "Title": "Start saving video stream to %0 with fps=%1 and codecId=%2",
            "Parameters": [
                ["Text","Filename Path"],
				["Integer","Frame rate (FPS)"],
				["Integer","Codec Id (-1 for user choice, otherwise see the list in help file)"]
			]
        },
		{
            "Title": "Stop saving video stream"
        },
		{
            "Title": "Set Background Color to %0",
			"Parameters": [
                ["Color","Background Color"]
			]
        },
		{
            "Title": "Load object classifier from %0",
            "Parameters": [
                ["Text","Filename Path"]
			]
        },
		{
            "Title": "Start tracking object"
        },
		{
            "Title": "Stop tracking object"
        },
		{
            "Title": "Load feature object from %0",
            "Parameters": [
                ["Text","Filename Path"]
			]
        },
		{
            "Title": "Start tracking feature"
        },
		{
            "Title": "Stop tracking feature"
        },
		{
            "Title": "Mirror Image"
        },
		{
            "Title": "Set Resolution %0x%1",
            "Parameters": [
                ["Integer","width"],
				["Integer","height"]
			]
        },
		{
            "Title": "Set Size %0x%1",
            "Parameters": [
                ["Integer","width"],
				["Integer","height"]
			]
        },
		{
            "Title": "Save raw frame at resolution to %0 ",
            "Parameters": [
                ["Text","Filename Path"]
			]
        },
		{
            "Title": "Set camera %0 to %1",
            "Parameters": [
				["Text","Property name (see helpfile for the whole list of property available)"],
                ["Float","Property value"]
			]
        },
		{
            "Title": "Start tracking barcode"
        },
		{
            "Title": "Stop tracking barcode"
        },
		{
            "Title": "Enable auto detection"
        },
		{
            "Title": "Disable auto detection"
        },
		{
            "Title": "Enable sharpening"
        },
		{
            "Title": "Disable sharpening"
        },
		{
            "Title": "Set Orientation %0",
			"Parameters": [
                ["Integer","Orientation type (0=Horizontal, 1=Vertical"]
			]
        }
    ],
	"Conditions": [
		{
            "Title": "%o : Is device initializing?",
            "Triggered": false
        },
        {
            "Title": "%o : Is device runing?",
            "Triggered": false
        },
		{
            "Title": "Has %o an error?",
            "Triggered": false
        },
		{
            "Title": "%o: Barcode found?",
            "Triggered": false
        }
    ],
    "Expressions": [
        {
            "Title": "LastError$(",
            "Returns": "Text"
        },
		{   "Title": "ColorPosX(",
            "Returns": "Integer",
            
            "Parameters":
            [
                ["Integer", "Color Id (1 or 2)"]
            ]
        },
		{   "Title": "ColorPosY(",
            "Returns": "Integer",
            
            "Parameters":
            [
                ["Integer", "Color Id (1 or 2)"]
            ]
        },
		{   "Title": "ColorHSVOnPos(",
            "Returns": "Integer",
            
            "Parameters":
            [
                ["Integer", "Position X"],
				["Integer", "Position Y"],
				["Integer", "Color Index (Hue=1, Saturation=2, Value=3) "]
            ]
        },
		{   "Title": "ColorRGBOnPos(",
            "Returns": "Integer",
            
            "Parameters":
            [
                ["Integer", "Position X"],
				["Integer", "Position Y"],
				["Integer", "Color Index (Red=1, Green=2, Blue=3)"]
            ]
        },
		{   "Title": "TrackColorObjectWidth(",
            "Returns": "Integer",
            
            "Parameters":
            [
                ["Integer", "Color Id (1 or 2)"]
            ]
        },
		{   "Title": "TrackColorObjectHeight(",
            "Returns": "Integer",
            
            "Parameters":
            [
                ["Integer", "Color Id (1 or 2)"]
            ]
        },
		{   "Title": "ObjectPosX(",
            "Returns": "Integer"
		},
		{   "Title": "ObjectPosY(",
            "Returns": "Integer"
		},
		{   "Title": "ObjectWidth(",
            "Returns": "Integer"
		},
		{   "Title": "ObjectHeight(",
            "Returns": "Integer"
		},
		{   "Title": "NumberOfGoodMatches(",
            "Returns": "Integer"
		},
		{   "Title": "FeatureCornerPosX(",
            "Returns": "Integer",
            
            "Parameters":
            [
                ["Integer", "Corner Index (1 to 4)"]
            ]
        },
		{   "Title": "FeatureCornerPosY(",
            "Returns": "Integer",
            
            "Parameters":
            [
                ["Integer", "Corner Index (1 to 4)"]
            ]
        },
		{
            "Title": "BarcodeText$(",
            "Returns": "Text"
        }
    ]
    
} &       �� ��     0	           M F 2                               �      �� ���    0	         M a i n   P a r a m e t e r s  F l i p   c a m e r a   i m a g e   ( h o r i z o n t a l ) " F l i p   h o r i z o n t a l l y   t h e   c a m e r a   i m a g e  B a c k g r o u n d   c o l o r  S e t   t h e   b a c k g r o u n d   c o l o r  D e b u g  S h o w   t r a c k i n g   o b j e c t s $ D i s p l a y   a   s h a p e   o n   t h e   o b j e c t s   f o u n d                   