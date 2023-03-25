#define PA_Addr		0x270
#define PB_Addr		0x271
#define PC_Addr		0x272
#define	CON_Addr	0x273

#define OPEN_ID 0x1
#define MODIFY_ID 0x2
#define ADMIN_ID 0x3
#define INIT_ID 0x4
#define INPUT_ID 0x5
#define SUCCESS 0x6
#define FAILURE 0x7
#define LOCK 0x8

#define u8	unsigned char 
#define u16	unsigned int 

extern char inportb( unsigned int );								//读I/O
extern void outportb( unsigned int, char);

struct input
{
	/* data */
	u8 input[8];
	u8 length;
	/*spe_order
		INPUT_ID:end without input and next is to reinput
		MODIFY_ID:end without input and next is to modify
		ADMIN_ID:end without input and next is to admin
	*/
	u8 spe_order;
};


u8 buffer[8] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};	//置显示缓冲器初值
u8 access_secret_origin[6]={0,1,2,4,5,6};//origin secret of access: 123456
u8 access_secret[6]={0,1,2,4,5,6};
u8 admin_secret[8]={0,1,2,4,5,6,8,9};//... of admin: 12345678
u8 count=0;//account of the failure attempt
u8 pos =0;

void delay(u16 ms);
void DIR();
u8 AllKey();
u8 shiftDown();
u8 key();
u8 init_infa();
u8 open_infa();
u8 modify_infa();
u8 admin_infa();
u8 access();
u8 access_verify(struct input in);
struct input input_infa(u8 caller_id);
void lcd_text();
void red();
void green();
void yellow();

void main()
{
	outportb(CON_Addr, 0x81);	//PA、PB输出，PC输入
	LCD_INIT();
	red();
	init_infa();

}

u8 init_infa(){
	/*
	screen show：please enter a command
	*/
	u8 com;
	u8 id;
	
	while(1)
	{
		draw("init mode");
		com=key();
		//显示输入的字符
		buffer[pos++]=com;
		pos&=0x7;
		DIR();
		
		switch (com)
		{
			case 7: //open
				id = open_infa();
				while(id!=INIT_ID){
					if(id==MODIFY_ID)
						id=modify_infa();
					else if(id==ADMIN_ID)
						id=admin_infa();
					else if(id==OPEN_ID)
						id=open_infa();
				}
				break;
			case 3:
				id = modify_infa();
				while(id!=INIT_ID){
					if(id==MODIFY_ID)
						id=modify_infa();
					else if(id==ADMIN_ID)
						id=admin_infa();
					else if(id==OPEN_ID)
						id=open_infa();
				}
				break;
			case 11:
				id = admin_infa();
				while(id!=INIT_ID){
					if(id==MODIFY_ID)
						id=modify_infa();
					else if(id==ADMIN_ID)
						id=admin_infa();
					else if(id==OPEN_ID)
						id=open_infa();
				}
				break;
			default:
					
				break;
		}		
	}
}

u8 access(u8 caller){
	/*
	caller OPNE_ID:	the secret is access_secret and the length is 6
		   MODIFY_ID:	the secret is access_secret and the length is 6
		   ADMIN_ID: 	the secret is admin_secret and the length is 8			
	return INIT_ID: normally finish open exit and call init
		   MODIFY_ID:get an modifykey when inputting the key exit and call admin
		   ADMIN_ID:get an adminkey when inputting the key exit and call admin
		   OPEN_ID:get an openkey when...

		   SUCCESS:
		   FAILURE:
		   LOCK:
	screen show: you are using open command
	*/
	struct input codegot;
	codegot = input_infa(caller);
	
	while(codegot.spe_order==INPUT_ID){//loop until not reinput
		draw("reinputing");
		codegot = input_infa(caller);
	}
	if(codegot.spe_order != 0){//lead to other command
		return codegot.spe_order;
	}

	//verify the code gotten: codegot.input[6] and secret
	if(access_verify(codegot)){//succeed opening the lock
		count=0;
		return SUCCESS;
		
	}else{
		count++;
		count &= 0x3;//avoid buffer-attack
		if( count <= 2 ){//attempt times less than 3
			return FAILURE;
		}else{//have alread fail 3 times
			return LOCK;
		}
	}
}

u8 open_infa(){
	/*
	return INIT_ID: normally finish open exit and call init
		   MODIFY_ID:get an modifykey when inputting the key exit and call admin
		   ADMIN_ID:get an adminkey when inputting the key exit and call admin
		   OPEN_ID:get an openkey when...
	screen show: you are using open command
	*/
	u8 sta = access(OPEN_ID);
	
	draw("open command");
	DIR();
	if(sta == SUCCESS){
		/*
		success! opening the door
		*/
		draw("success opening the door");
		delay(3000);
		green();
		delay(10000);
		red();
	}else if(sta == FAILURE){
		/*
		failure! you still have %d chances 
		*/
		if(count ==1)
		{
			draw("failure: you have tried one time");
		}
		else if(count ==2)
		{
			draw("failure: you have tried two times");	
		}

	}else if(sta == LOCK){
		/*
		failure! you have tried three times! you are locked
		*/
		draw("locked you have tried three times");
		yellow();
		red();
	}else{
		return sta;
	}
	return INIT_ID;
}

u8 modify_infa(){
	/*
	return INIT_ID: normally finish open exit and call init
		   MODIFY_ID:get an modifykey when inputting the key exit and call admin
		   ADMIN_ID:get an adminkey when inputting the key exit and call admin
		   OPEN_ID:get an openkey when...	
	screen show: you are using modify command
	*/
	struct input temp;
	u8 i;
	u8 sta;
	
	draw("modify command enter old secret to change secret");
	sta = access(MODIFY_ID);
	if(sta == SUCCESS){
		/*
		success! 
		*/
		draw("verify success please enter your new secret");
		delay(4000);
	}else if(sta == FAILURE){
		/*
		failure! you still have %d chances 
		*/
		if(count ==1)
		{
			draw("failure: you have tried one time");
		}
		else if(count ==2)
		{
			draw("failure: you have tried two times");	
		}
		
		return INIT_ID;
	}else if(sta == LOCK){
		/*
		failure! you have tried three times! you are locked
		*/
		draw("locked");
		return INIT_ID; 
	}else{
		return sta;
	}

	//succeed in verify
	//please enter your new secret
	temp = input_infa(MODIFY_ID);
	
	while(temp.spe_order==INPUT_ID){//loop until not reinput
		draw("reinouting");
		temp = input_infa(MODIFY_ID);
	}
	if(temp.spe_order != 0){//lead to other command
		return temp.spe_order;
	}

	//normally get input. update the secret
	//updating the secret!
	draw("updating the secret");
	delay(2000);
	for(i=0;i<6;i++){
		access_secret[i]=temp.input[i];
	}
	draw("update completed");
	delay(3000);
	//succeed in updating the secret
	return INIT_ID;
}

u8 admin_infa(){
	/*
	if next need to call open: return 1
						 modify: return 2
						 admin: return 3
						 init_state: return 4
	screen show: you are using admin command
	*/
	struct input temp;
	u8 sta;
	u8 i;

	draw("admin command please enter the secret of amdin");
	
	sta = access(ADMIN_ID);
	if(sta == SUCCESS){
		/*
		success! 
		*/
		draw("verify admin success i am resetting your secret");
		delay(4000);
	}else if(sta == FAILURE){
		/*
		failure! you still have %d chances 
		*/
		if(count ==1)
		{
			draw("failure: you have tried one time");
		}
		else if(count ==2)
		{
			draw("failure: you have tried two times");	
		}
		
		return INIT_ID;
	}else if(sta == LOCK){
		/*
		failure! you have tried three times! you are locked
		*/
		draw("locked");
		return INIT_ID; 
	}else{
		return sta;
	}
	
	//succeed in verify
	//reset secret
	for(i=0;i<6;i++)
		access_secret[i]=access_secret_origin[i];
	draw("reset completed");
	return INIT_ID;
}

struct input input_infa(u8 caller_id){
	/*
		get: caller= OPEN_ID: caller is open 
			 caller= MODIFY_ID: caller is modify
			 caller= ADMIN_ID: caller is admin
		
		return: temp 
		attribuite spe_order of temp points out what to do after exited this function 
		input.spe_order: 0/INPUT_ID/OPEN_ID/MODIFY_ID/ADMIN_ID/INIT_ID  
		0 means exit normally
		INPUT_ID means reinput
		-----
		attribuite length of temp points out the program is running in which mode
		6 means open or modify
		8 means admin
	*/
	
	u8 ch1;
	u8 i;
	u8 n;
	struct input temp;
	temp.spe_order=0;
	n=0;
	if(caller_id ==OPEN_ID ||caller_id == MODIFY_ID){	//6bit input
		/*
		screen show: please enter a 6-bit phrase
		*/
		draw("please enter a six secret");
		temp.length=6;
		i=0;
		while(1){
			ch1 = key();
			n++;
			if(n<=6)
				display_star(n);	
			buffer[pos++]=ch1;
			pos&=0x7;
			DIR();
			if(ch1==13){	//ch1 == '#'
				if(i == 6){//i range 0-6
					//end normaly
					temp.spe_order=0;// end normally with input
					return temp;	
				}else{//when i<6
					temp.spe_order=INIT_ID;
					return temp;
				}
			}else if(ch1==14){	// 'del'
				temp.spe_order=INPUT_ID;//end without input and call reinput
				return temp;
			}else if(ch1==7){//'open'
				temp.spe_order=OPEN_ID;//end without input and call open
				return temp;
			}else if(ch1==3){//'modify'
				temp.spe_order=MODIFY_ID;//end without input and call modify
				return temp;
			}else if(ch1==11){//'admin'
				temp.spe_order=ADMIN_ID;//end without ... and call admin 
				return temp;
			}else{//others
				//show this number or char 
				if(i<6){
					temp.input[i++]=ch1;
				}else{
					/*
					[optional] show: warning! most 6 bits
					*/
					draw("warning most six char");
				}
			}
			
		}	
	}else if(caller_id ==ADMIN_ID){//8 bit input
		/*
		screen show: please enter a 8-bit phrase
		*/
		draw("please enter a eight secret");
		temp.length=8;
		i=0;
		while(1){
			ch1 = key();
			n++;
			if(n<=8)
				display_star(n);	
			buffer[pos++]=ch1;
			pos&=0x7;
			DIR();
			if(ch1==13){	//ch1 == '#'
				if(i == 8){//i range 0-8
					//end normaly
					temp.spe_order=0;// end normally with input
					return temp;	
				}else{//when i<8
					temp.spe_order=INIT_ID;
					return temp;
				}
			}else if(ch1==14){	// 'del'
				temp.spe_order=INPUT_ID;//end without input and call reinput
				return temp;
			}else if(ch1==7){//'open'
				temp.spe_order=OPEN_ID;//end without input and call open
				return temp;
			}else if(ch1==3){//'modify'
				temp.spe_order=MODIFY_ID;//end without input and call modify
				return temp;
			}else if(ch1==11){//'admin'
				temp.spe_order=ADMIN_ID;//end without ... and call admin 
				return temp;
			}else{//others
				//show this number or char 
				if(i<8){
					temp.input[i++]=ch1;
				}else{
					/*
					[optional] show: warning! most 8 bits
					*/
					draw("warning most eight char");
				}
			}
			
		}	
	}
	return temp;
}


u8 access_verify(struct input in){
	//compare in.input[] with access_secret 
	//the length of secret is recorded in in.length
	u8 i=0;
	if(in.length==6){
		for(i=0;i<6;i++){
			if(in.input[i]!=access_secret[i]){
				return 0;
			}
		}
		return 1;
	}else{
		for(i=0;i<8;i++){
			if(in.input[i]!=admin_secret[i]){
				return 0;
			}
		}
		return 1;
	}
	
}


void lcd_text(){
	//无条件实现所有数码管全灭5秒后全亮
	outportb(PA_Addr, 0xff);		//段数据
	outportb(PB_Addr, 0x0);						//选择数码管
	delay(30000);									//延迟3ms
	outportb(PB_Addr, 0x0);

}
void delay(u16 ms)
{
	u16 i;
	while(ms--)
	{
		i = 100;
		do
		{;}while(--i);
	}
}

//8个数码管都刷新显示
void DIR()
{
	u8 i, dig = 0x7f;
    //dp g.. d c b a/0 有效
    /*
    0   1   2   3
    --------------------
    1/a 2/b 3/c modify
    4/d 5/e 6/f open
    7/g 8/h 9/i admin
    0/j #   del shift    
    */
    //             1    2    3    modi 4    5    6    open 7    8    9    adm  0    #    del  shif
    u8 SegArray[]={0xf9,0xa4,0xb0,0xf0,0x99,0x92,0x82,0xf1,0xf8,0x80,0x90,0xf2,0xc0,0xf3,0xf4,0xf5,
    //             a    b    c    modi d    e    f    open g    h    i    adm  j    #    del  shif    
				   0x88,0x83,0xc6,0xf6,0xa1,0x86,0x8e,0xf7,0xc2,0x89,0xf0,0xf8,0xf1,0xf9,0xfa,0xf5};
	for (i = 0 ; i < 8; i++)
	{
		outportb(PA_Addr, SegArray[buffer[i]]);		//段数据
		outportb(PB_Addr, dig);						//选择数码管
		delay(3);									//延迟3ms
		outportb(PB_Addr, 0xff);
		dig = ((dig >> 1) | 0x80);
	}
}

//检查是否有按键按下，若有返回1
u8 AllKey()
{
	u8 i;
	outportb(PB_Addr, 0x0);
	i = (~inportb(PC_Addr) & 0x3);
	return i;
}

//shift按下返回1
u8 shiftDown()
{
    u8 i;
    outportb(PB_Addr,0x7f);
    i = (~inportb(PC_Addr) & 0x2);
    return i;
}

//返回按下的按键的编码
//原16个按键1-16，shift加0-9为17-26
u8 key()
{
	u8 i, j, keyResult;
    u8 i2,j2,keyResult2;
	u8 bNoKey = 1;
//	u8 bNoKey2 = 1;
	
    while(bNoKey)
	{
		if (AllKey() == 0)		//调用判有无闭合键函数
		{
			DIR();
			DIR();
			continue;
		}
		DIR();
		DIR();
		if (AllKey() == 0)		//调用判有无闭合键函数
			continue;
		
		i = 0xfe;
		keyResult = 0;
		do
		{
			outportb(PB_Addr, i);
			j = ~inportb(PC_Addr);
			if (j & 3)
			{
				bNoKey = 0;
				if (j & 2)				//after行有键闭合
					keyResult += 8;
			}
			else						//没有键按下
			{
				keyResult++;			//列计数器加1
				i = ((i << 1) | 1);
			}
		}while(bNoKey  && (i != 0xff));

	}
	
	if (!bNoKey)
	{
            
            while(AllKey())		//判断释放否
		    {
		    	if(shiftDown())
		    	{
		    		keyResult += 16;
		    		while(AllKey())
		    		{
		    			DIR();
	    			}
	    			break;		
	    		}
		    	DIR();
		    }
	}

	return keyResult;
}
void red()  //红灯亮
{
	//outportb(CON_Addr, 0x80);	
	outportb(PC_Addr, 0xdf);
}
void green()  //绿灯亮
{
	//outportb(CON_Addr, 0x80);	
	outportb(PC_Addr, 0x7f);
}
void yellow()
{
		u8 i;
		//outportb(CON_Addr, 0x80);	
		for (i = 0; i < 6; i++)
		{
			outportb(PC_Addr, 0xbf);		//黄灯闪烁亮
			delay(500);
			outportb(PC_Addr, 0xff);
			delay(500);
		}
}


