//////////////////// OPTICS /////////////////////

//Soviet dovetails
class AttachmentOpticsDovetailClass {}
AttachmentOpticsDovetailClass AttachmentOpticsDovetailSource;

class AttachmentOpticsDovetail : AttachmentOptics
{
};

//Dovetail - RPG7 
class AttachmentOpticsDovetailRPG7Class {}
AttachmentOpticsDovetailRPG7Class AttachmentOpticsDovetailRPG7Source;

class AttachmentOpticsDovetailRPG7 : AttachmentOpticsDovetail
{
};

//Dovetail - AK/SVD
class AttachmentOpticsDovetailAKSVDClass {}
AttachmentOpticsDovetailAKSVDClass AttachmentOpticsDovetailAKSVDSource;

class AttachmentOpticsDovetailAKSVD : AttachmentOpticsDovetail
{
};

//Dovetail - AK
class AttachmentOpticsDovetailAKClass {}
AttachmentOpticsDovetailAKClass AttachmentOpticsDovetailAKSource;

class AttachmentOpticsDovetailAK : AttachmentOpticsDovetailAKSVD
{
};

//Dovetail - SVD
class AttachmentOpticsDovetailSVDClass {}
AttachmentOpticsDovetailSVDClass AttachmentOpticsDovetailSVDSource;

class AttachmentOpticsDovetailSVD : AttachmentOpticsDovetailAKSVD
{
};

//UK 59 dovetail
class AttachmentOpticsDovetailUK59Class {}
AttachmentOpticsDovetailUK59Class AttachmentOpticsDovetailUK59Source;

class AttachmentOpticsDovetailUK59 : AttachmentOptics
{
};

//M21 scope mount
class AttachmentOpticsM21Class {}
AttachmentOpticsM21Class AttachmentOpticsM21Source;

class AttachmentOpticsM21 : AttachmentOptics
{
};




//M16 carryhandle mount
class AttachmentOpticsCarryHandleClass {}
AttachmentOpticsCarryHandleClass AttachmentOpticsCarryHandleSource;

class AttachmentOpticsCarryHandle : AttachmentOptics
{
};



// RIS 1913 - long pic rail
// Rail for optical accessories. Length of this rail is above or equal 250mm 
// Optics like scope + night vision sights combos can be attached to it
class AttachmentOpticsRIS1913Class {}
AttachmentOpticsRIS1913Class AttachmentOpticsRIS1913Source;

class AttachmentOpticsRIS1913 : AttachmentOptics
{
};

// RIS 1913 - medium pic rail 
// Rail for optical accessories. Length of this rail is up to 250mm 
// Optics like larger scopes, collimator + magnifier combos or similar can be attached to it
class AttachmentOpticsRIS1913MediumClass {}
AttachmentOpticsRIS1913MediumClass AttachmentOpticsRIS1913MediumSource;

class AttachmentOpticsRIS1913Medium : AttachmentOpticsRIS1913
{
};

// RIS 1913 - pic rail short
// Rail for optical accessories. Length of this rail is up to 120mm 
// Optics like collimators, scopes or similar can be attached to it
class AttachmentOpticsRIS1913ShortClass {}
AttachmentOpticsRIS1913ShortClass AttachmentOpticsRIS1913ShortSource;

class AttachmentOpticsRIS1913Short : AttachmentOpticsRIS1913Medium
{
};

// RIS 1913 - pic rail very short
// Rail for optical accessories. Length of this rail is up to 80mm 
// Optics like small collimators, ris ironsights or similar can be attached to it
class AttachmentOpticsRIS1913VeryShortClass {}
AttachmentOpticsRIS1913VeryShortClass AttachmentOpticsRIS1913VeryShortSource;

class AttachmentOpticsRIS1913VeryShort : AttachmentOpticsRIS1913Short
{
};
