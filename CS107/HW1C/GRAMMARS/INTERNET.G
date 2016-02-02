Subject: RSG grammar: Internet Press Releases!
Date: Mon, 26 Apr 1999 01:44:01 -0700
x-sender: nsanjana@nsanjana.pobox.stanford.edu
x-mailer: Claris Emailer 2.0v3, January 22, 1998
From: Neville Sanjana <nsanjana@stanford.edu>
To: <zelenski@CS.Stanford.EDU>

Hi Julie,

     While testing my RSG, I decided to make a small experimental 
grammar, which slowly grew into a large grammar. Although utterly useless 
in almost all respects (except perhaps for really desperate IE majors), 
the Amazing Internet Start-Up Press Release Generator is ready for 
prime-time.

     This grammar creates press releases for hypothetical products, 
following on the heels of the recent Internet craze by Wall Street and 
the media at large. It is funny (sometimes).

---------------------------------------
The Amazing Internet Start-Up Press Release Generator
by Neville Sanjana (nsanjana@stanford.edu)
Date: April 25, 1999

{
<start>

<flashy-headline>

For Immediate Release:
	<lies-about-product>

	<end-grabber>
<contact>	;
}

{
<flashy-headline>

<product-name> Will Rock Your World! ;
<product-name> Lets You Check Email and Surf the Web... at the same time! ;
<product-name> Puts the Entire Internet in the Palm of Your Hand! ;
<product-name> Will Teach You Java in <ridiculously-small-time> ! ;
<product-name> Will Let You Launch Your Own Internet Start-Up! ;
<product-name> Will Make You a Better Person! ;
<product-name> Will Let You Check Email While You're Asleep! ;
<product-name> Will Give You Supernatural Powers and Check Your Email! ;

}

{
<lies-about-product>

<quick-quip>

<description1> <description2> <description3> ;
}

{
<description1>
It might surprise you at first, but soon you will learn how <general-name-l> is going to change your world. ;
Life will never be quite the same after you experience <general-name-l> . ;
You've used the Internet, but you weren't satisfied. That's where <general-name-l> can help. ;
You live online, you sleep online, you eat online. You're always online... <general-name-l> is just for you. ;

}

{
<description2>

<general-name> employs amazing new technology to revolutionize the way you do business. ;
<general-name> is the soon-to-be de facto standard for business and home telecommunications and intra-/inter-networking. ;
<general-name> affects every aspect of the modern world: If there's one product to get right now, it's this one. ;
<general-name> is indispensible and, best of all, it's free. We make money through advertising revenue, but our focus is always on the customer. ;
No one can really live without <general-name-l> . That's why we invented it. ;
}


{
<description3>

Surprisingly, <general-name-l> appeals to every market segment. ;
<general-name> might even help with your relationships... but, hey, no promises on that one. ;
<general-name> is witty, biting, and fresh. It's like a cross between Marilyn Manson and Burt Bacharach. ;
<general-name> is going to sell like hotcakes. Run to the store now. Go. Tell all your friends. Go. Now. ;
}

{
<end-grabber>
Buy it: it's your moral duty. ;
Buy it: it is your destiny. ;
Remember, our product is just like the Internet, but better. ;
Do it for your country: support our Internet company. ;
Presently, <general-name-l> is in development and will not be available until <long-time> . ;
<general-name> is currently in beta. But, we will release after <long-time> . ;
}

{
<general-name>

This product ;
}

{
<general-name-l>

this product ;
}

{
<quick-quip>

It's a miracle! ;
The revolution is here! ;
The time has come. ;
Join the revolution. ;
Get ready to see the future. ;
Welcome to the digital age. ;
Welcome to the electronic age. ;
Welcome to your new life. ;
Think Different. ;
Where do you want to go tomorrow? ;
}

{
<product-name>

<product> <trademark> ;
}

{
<product>
Internet Rock ;
Internet-in-a-Can ;
Virtual Restaurant ;
Internet Cereal ;
CS 106 Section Leader Blow-Up  Doll ;
Learn to Think in Java ;
Internet Surgeon: Click Your Way to Health ;
WooHoo!: The Newest Internet Search Engine ;
Internet Shampoo ;
}

(InternetShampoo idea, courtesy of Jane Wong!)

{
<contact>

We are a virtual corporation, and therefore have no contact information at this time. ;
We can be reached at:  (555) 456-7890  (ask for Moe) ;
For more information, email Hans at: info@pyramidscheme.com ;
Please feel free to contact any members of our (virtual) Internet marketing team: info@flybynight.com ;
Contact information:  (999) 456-7890  (ask for Pauly, but don't leave any messages if the pizza guy picks up) ;
}

{
<trademark>

(tm) ;
(reg) ;
(c) ;
(patent pending) ;
}

{
<ridiculously-small-time>

32 picoseconds ;
one half-life of a cesium atom ;
6.02 x 10^23 microseconds ;
2 minutes ;
}

{
<long-time>

hell freezes over ;
the second coming ;
the revolution of the proletariat ;
the Y10K bug hits ;
the next millennium ;
}
