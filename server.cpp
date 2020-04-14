// Server side C program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <aws/core/Aws.h>
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/InvokeRequest.h>
#include <iostream>
#include <fstream>
#include <sstream>

// #include <boost/asio.hpp> 

#define PORT 8090

void InvokeFunction_lambda(Aws::String functionName,char* message)
{
    // printf("%s\n", message);
    Aws::Lambda::Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(functionName);
    invokeRequest.SetInvocationType(Aws::Lambda::Model::InvocationType::RequestResponse);
    std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("Test");
    *payload << message;
    invokeRequest.SetBody(payload);
    Aws::Lambda::LambdaClient m_client;
    auto outcome = m_client.Invoke(invokeRequest);

    if (outcome.IsSuccess())
    {
        auto &result = outcome.GetResult();

        // Lambda function result (key1 value)
        Aws::IOStream& payload = result.GetPayload();
        Aws::String functionResult;
        std::getline(payload, functionResult);
        std::cout << "Lambda result:\n" << functionResult << "\n\n";

    }
    else
    {
      std::cout << "Error while invoking function " << outcome.GetError().GetMessage()
        << std::endl;
    }
}

void push_sns(Aws::String topic_arn,char* message)
{
    // printf("%s\n", message);
    Aws::SNS::SNSClient sns;
    Aws::SNS::Model::PublishRequest psms_req;
    psms_req.SetMessage(message);
    psms_req.SetTopicArn(topic_arn);

    auto psms_out = sns.Publish(psms_req);

    if (psms_out.IsSuccess())
    {
      std::cout << "Message published successfully " << std::endl;
    }
    else
    {
      std::cout << "Error while publishing message " << psms_out.GetError().GetMessage()
        << std::endl;
    }
}

int main(int argc, char* argv[]) 
{ 
    
    
    if(argv[1]==NULL)
    {
        std::cout << "Please provide required aws service SNS | Lambda" << std::endl ;
        exit(EXIT_FAILURE);
    }
    std::string argv1 = argv[1];
    if(argv[2]==NULL)
    {
        std::cout << "Please provide Arn address" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string argv2 = argv[2];
    if(argv1!= "sns" && argv1!= "lambda" )
    {
        std::cout << "Please provide valid arguments" << std::endl ;
        exit(EXIT_FAILURE);
    }
    
    char* arn = argv[2];
    //Should build an arn parser//

    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[3000]={0};
    char *return_continue = "HTTP/1.1 100 Continue";
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n-----------------Listening notifications---------------------\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        buffer[3000] = {0};
        valread = read( new_socket , buffer, 3000);
        printf("%s\n",buffer );
        write(new_socket , return_continue , strlen(return_continue));
        buffer[3000] = {0};
        valread = read( new_socket , buffer, 30000);
        printf("%s\n",buffer );
        Aws::SDKOptions options;
        Aws::InitAPI(options);
        {
            if(argv[1]=="sns")
            {
                push_sns(arn,buffer);
            }
            else 
            {
                InvokeFunction_lambda(arn,buffer);
            }

            Aws::ShutdownAPI(options);
        }
    }
    return 0; 
} 

    


